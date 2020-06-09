// SPDX-License-Identifier: Apache-2.0
#include "config.h"

#include "guard_interface.hpp"

#include "guard_common.hpp"
#include "guard_entity.hpp"
#include "guard_file.hpp"
#include "guard_log.hpp"
#include "include/guard_record.hpp"

#ifdef DEV_TREE
#include "phal_devtree.hpp"
#endif /* DEV_TREE */

#include <cstring>

namespace openpower
{
namespace guard
{

using namespace openpower::guard::log;

static fs::path guardFilePath = "";

#ifdef PGUARD
static constexpr size_t headerSize = 0;
#else
static constexpr size_t headerSize = 16;
#endif

void initialize()
{
    if (guardFilePath.empty())
    {
        if (!fs::exists(GUARD_PRSV_PATH))
        {
            if (fs::exists(GUARD_RO_PATH))
            {
                fs::copy_file(GUARD_RO_PATH, GUARD_PRSV_PATH);
            }
            else
            {
                guard_log(GUARD_ERROR, "Fail to find guard file %s",
                          GUARD_RO_PATH);
            }
        }
        guardFilePath = GUARD_PRSV_PATH;
    }
#ifndef PGUARD
    // validate magic number, read from 0th position
    GuardRecord_t guardRecord;
    GuardFile file(guardFilePath);
    file.read(0, &guardRecord, sizeof(guardRecord));
    if (strncmp((char*)guardRecord.iv_magicNumber, GUARD_MAGIC,
                sizeof(guardRecord.iv_magicNumber)) != 0)
    {
        size_t headerPos = 8;
        guard_log(
            GUARD_INFO,
            "Updating magic number and guard version to the GUARD partition.");
        memcpy((char*)guardRecord.iv_magicNumber, GUARD_MAGIC,
               sizeof(guardRecord.iv_magicNumber));
        file.write(0, &guardRecord.iv_magicNumber,
                   sizeof(guardRecord.iv_magicNumber));
        guardRecord.iv_version = CURRENT_GARD_VERSION_LAYOUT;
        file.write(headerPos, &guardRecord.iv_version,
                   sizeof(guardRecord.iv_version));
    }
#endif
}

bool isBlankRecord(const GuardRecord& guard)
{
    GuardRecord blankRecord;
    memset(&blankRecord, 0xff, sizeof(guard));
    return (memcmp(&blankRecord, &guard, sizeof(guard)) == 0);
}

int guardNext(GuardFile& file, int pos, GuardRecord& guard)
{
    uint32_t offset = (pos * sizeof(guard)) + headerSize;
    uint32_t size = file.size();
    if (offset > size)
    {
        return -1;
    }
    memset(&guard, 0, sizeof(guard));
    file.read(offset, &guard, sizeof(guard));
    if (isBlankRecord(guard))
    {
        return -1;
    }
    return pos;
}

#define for_each_guard(file, pos, guard)                                       \
    for (pos = guardNext(file, 0, guard); pos >= 0;                            \
         pos = guardNext(file, ++pos, guard))

void create(const EntityPath& entityPath, uint32_t eId, uint8_t eType)
{

    //! check if guard record already exists
    int pos = 0;
    int lastPos = 0;
    uint32_t maxId = 0;
    GuardRecord existGuard;
    memset(&existGuard, 0xff, sizeof(existGuard));

    GuardFile file(guardFilePath);
    for_each_guard(file, pos, existGuard)
    {
        if (existGuard.targetId == entityPath)
        {
            guard_log(
                GUARD_ERROR,
                "Already guard record is available in the GUARD partition");
            return;
        }
        //! find the largest record ID
        if (be32toh(existGuard.recordId) > maxId)
        {
            maxId = be32toh(existGuard.recordId);
        }
        lastPos++;
    }

    //! if blank record exist
    if (isBlankRecord(existGuard))
    {
        GuardRecord guard;
        uint32_t offset = lastPos * sizeof(guard);
        memset(&guard, 0xff, sizeof(guard));
        guard.recordId = htobe32(maxId + 1);
        guard.errType = eType;
        guard.targetId = entityPath;
        guard.elogId = htobe32(eId);
        //! TODO:- Need to fetch details from device tree APIs i.e. serial
        //! number and part number.
        // For now initializing serial number and part number with 0.
#ifndef PGUARD
        memset(guard.u.s1.serialNum, 0, sizeof(guard.u.s1.serialNum));
        memset(guard.u.s1.partNum, 0, sizeof(guard.u.s1.partNum));
#endif
        uint32_t guardFileSize = file.size();
        if (offset > (guardFileSize - sizeof(guard)))
        {
            guard_log(
                GUARD_ERROR,
                "GUARD file has no space to write the new record in the file.");
            throw std::runtime_error("No space in GUARD for a new record");
        }
        file.write(offset + headerSize, &guard, sizeof(guard));
    }
    else
    {
        guard_log(
            GUARD_ERROR,
            "GUARD file has no space to write the new record in the file.");
        std::runtime_error("No space in GUARD for a new record");
    }
}

GuardRecords getAll()
{
    GuardRecords guardRecords;
    GuardRecord curRecord;
    int pos = 0;
    GuardFile file(guardFilePath);
    for_each_guard(file, pos, curRecord)
    {
        curRecord.recordId = be32toh(curRecord.recordId);
        curRecord.elogId = be32toh(curRecord.elogId);
        guardRecords.push_back(curRecord);
    }
    return guardRecords;
}

void clear(const EntityPath& entityPath)
{
    int pos = 0;
    int delpos = 0;
    int lastPos = 0;
    GuardRecord existGuard;
    GuardRecord null_guard;
    bool found = false;
    memset(&null_guard, 0xFF, sizeof(null_guard));

    GuardFile file(guardFilePath);
    for_each_guard(file, pos, existGuard)
    {
        if (existGuard.targetId == entityPath)
        {
            delpos = pos;
            found = true;
        }
        lastPos++;
    }

    if (!found)
    {
        guard_log(GUARD_ERROR, "Guard record not found");
        return;
    }
    uint32_t offset = delpos * sizeof(null_guard);
    file.erase(offset + headerSize, sizeof(null_guard));

    int i = delpos + 1;
    while (lastPos != i)
    {
        uint32_t offset1 = i * sizeof(existGuard);
        file.read(offset1 + headerSize, &existGuard, sizeof(existGuard));
        uint32_t offset2 = (i - 1) * sizeof(existGuard);
        file.write(offset2 + headerSize, &existGuard, sizeof(existGuard));
        file.write(offset1 + headerSize, &null_guard, sizeof(null_guard));
        i++;
    }
}

void clearAll()
{
    GuardRecord guard;

    memset(&guard, 0, sizeof(guard));
    GuardFile file(guardFilePath);
    file.read(0 + headerSize, &guard, sizeof(guard));
    if (isBlankRecord(guard))
    {
        guard_log(GUARD_INFO, "No GUARD records to clear");
    }
    else
    {
        int pos = 0;
        file.erase(pos + headerSize, file.size());
    }
}

void libguard_init(bool guardInit)
{
    initialize();
    guard_log(GUARD_DEBUG, "guardInit value is %d", guardInit);
#ifdef DEV_TREE

    if (guardInit)
    {
        openpower::guard::log::guard_log(GUARD_INFO,
                                         "Using power system device tree");
        openpower::guard::phal::initPHAL();
    }

#endif /* DEV_TREE */
}

namespace utest
{
void setGuardFile(const fs::path& file)
{
    guardFilePath = file;
}
} // namespace utest
} // namespace guard
} // namespace openpower
