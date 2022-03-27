// SPDX-License-Identifier: Apache-2.0
#include "config.h"

#include "guard_interface.hpp"

#include "guard_common.hpp"
#include "guard_entity.hpp"
#include "guard_exception.hpp"
#include "guard_file.hpp"
#include "guard_log.hpp"
#include "include/guard_record.hpp"

#ifdef DEV_TREE
#include "phal_devtree.hpp"
#endif /* DEV_TREE */

#include <cstring>
#include <variant>

namespace openpower
{
namespace guard
{

using namespace openpower::guard::log;
using guardRecordParam = std::variant<EntityPath, uint32_t>;
using namespace openpower::guard::exception;

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
            guard_log(GUARD_ERROR, "Fail to find guard file %s",
                      GUARD_PRSV_PATH);
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

const fs::path& getGuardFilePath()
{
    if (guardFilePath.empty())
    {
        guard_log(GUARD_ERROR, "Guard file is not initialised.");
        throw GuardFileOpenFailed(
            "Guard file is not initialised. "
            "Please make sure libguard_init() is called already");
    }

    return guardFilePath;
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

/**
 * @brief Helper function to return guard record in host
 *        endianess format
 *
 * @param[in] record - bigendian record
 *
 * @return guard record in host endianness format
 *
 */
static GuardRecord getHostEndiannessRecord(const GuardRecord& record)
{
    GuardRecord convertedRecord = record;
    convertedRecord.recordId = be32toh(convertedRecord.recordId);
    convertedRecord.elogId = be32toh(convertedRecord.elogId);
    return convertedRecord;
}

#define for_each_guard(file, pos, guard)                                       \
    for (pos = guardNext(file, 0, guard); pos >= 0;                            \
         pos = guardNext(file, ++pos, guard))

GuardRecord create(const EntityPath& entityPath, uint32_t eId, uint8_t eType,
                   bool overwriteRecord)
{

    //! check if guard record already exists
    int pos = 0;
    int lastPos = 0;
    uint32_t offset = 0;
    uint32_t avalSize = 0;
    uint32_t maxId = 0;
    uint32_t id = 0;
    int empPos = -1;
    GuardRecord existGuard;
    GuardRecord guard;
    size_t sizeOfGuard = sizeof(guard);
    memset(&guard, 0xff, sizeOfGuard);
    memset(&existGuard, 0xff, sizeOfGuard);

    GuardFile file(guardFilePath);
    for_each_guard(file, pos, existGuard)
    {
        // Storing the oldest resolved guard record position.
        if ((existGuard.recordId == GUARD_RESOLVED) && (empPos < 0))
        {
            empPos = pos;
        }

        if (existGuard.targetId == entityPath)
        {
            /**
             * - Ignore the existing record if resolved
             * - Ignore ephemeral records (GARD_Reconfig and
             *   GARD_Sticky_deconfig) since the assumption is the host
             *   application created for their usage to support resource
             *   recovery and no one will create those types of records
             *   other than Hostboot and also they are using their own
             *   infrastructure for the guard operation, not the libguard.
             */
            if (existGuard.recordId == GUARD_RESOLVED ||
                existGuard.errType == GARD_Reconfig ||
                existGuard.errType == GARD_Sticky_deconfig)
            {
                lastPos++;
                continue;
            }
            else if (overwriteRecord)
            {
                if ((existGuard.errType == GARD_User_Manual) &&
                    ((eType == GARD_Fatal) || (eType == GARD_Predictive)))
                {
                    // Override the existing manual guard if the given record
                    // type is Fatal or Predictive
                    offset = lastPos * sizeOfGuard;
                    existGuard.errType = eType;
                    existGuard.elogId = htobe32(eId);
                    file.write(offset + headerSize, &existGuard, sizeOfGuard);
                }
                else if ((existGuard.errType == GARD_Predictive) &&
                         (eType == GARD_Fatal))
                {
                    // Override the existing Predictive guard if the given
                    // record type is Fatal
                    offset = lastPos * sizeOfGuard;
                    existGuard.errType = eType;
                    existGuard.elogId = htobe32(eId);
                    file.write(offset + headerSize, &existGuard, sizeOfGuard);
                }
                else
                {
                    guard_log(
                        GUARD_ERROR,
                        "Failed to overwrite since record is already exist and "
                        "that does not meet the condition to overwrite");
                    throw AlreadyGuarded(
                        "Failed to overwrite, Guard record is already exist");
                }
            }
            else
            {
                guard_log(
                    GUARD_ERROR,
                    "Already guard record is available in the GUARD partition");
                throw AlreadyGuarded("Guard record is already exist");
            }
            return getHostEndiannessRecord(existGuard);
        }

        id = be32toh(existGuard.recordId);
        //! find the largest record ID
        if ((id > maxId) && (existGuard.recordId != GUARD_RESOLVED))
        {
            maxId = be32toh(existGuard.recordId);
        }
        lastPos++;
    }

    // Space left in GUARD file before writing a new record
    avalSize = file.size() - ((lastPos + 1) * sizeOfGuard + headerSize);
    if (avalSize < sizeOfGuard)
    {
        if (empPos < 0)
        {
            guard_log(GUARD_ERROR,
                      "Guard file size is %d and space remaining in GUARD file "
                      "is %d\n",
                      file.size(), avalSize);
            throw GuardFileOverFlowed(
                "Enough size is not available in GUARD file");
        }
        // No space is left and have invalid record present. Hence using that
        // slot to write new guard record.
        offset = empPos * sizeOfGuard;
    }
    else
    {
        offset = lastPos * sizeOfGuard;
    }

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
    file.write(offset + headerSize, &guard, sizeOfGuard);

    return getHostEndiannessRecord(guard);
}

GuardRecords getAll(bool persistentTypeOnly)
{
    GuardRecords guardRecords;
    GuardRecord curRecord;
    int pos = 0;
    GuardFile file(guardFilePath);
    for_each_guard(file, pos, curRecord)
    {
        if (persistentTypeOnly && (curRecord.errType == GARD_Reconfig ||
                                   curRecord.errType == GARD_Sticky_deconfig))
        {
            continue;
        }
        curRecord.recordId = be32toh(curRecord.recordId);
        curRecord.elogId = be32toh(curRecord.elogId);
        guardRecords.push_back(curRecord);
    }
    return guardRecords;
}

/**
 * @brief To find the guard record based on recordId or entityPath
 *
 * @param[in] value (std::variant<EntityPath, uint32_t>)
 *
 * @return NULL on success.
 * 		   Throw following exceptions:
 * 		   -InvalidEntry
 * 		   -InvalidEntityPath
 *
 */
static void invalidateRecord(const guardRecordParam& value)
{
    int pos = 0;
    GuardRecord existGuard;
    bool found = false;
    uint32_t offset = 0;
    EntityPath entityPath = {};
    uint32_t recordPos = 0;

    if (std::holds_alternative<uint32_t>(value))
    {
        recordPos = std::get<uint32_t>(value);
    }
    else if (std::holds_alternative<EntityPath>(value))
    {
        entityPath = std::get<EntityPath>(value);
    }
    else
    {
        throw InvalidEntry(
            "Invalid parameter passed to invalidate guard record");
    }

    GuardFile file(guardFilePath);
    for_each_guard(file, pos, existGuard)
    {
        if (((be32toh(existGuard.recordId) == recordPos) ||
             (existGuard.targetId == entityPath)) &&
            (existGuard.recordId != GUARD_RESOLVED))
        {
            offset = pos * sizeof(existGuard);
            existGuard.recordId = GUARD_RESOLVED;
            file.write(offset + headerSize, &existGuard, sizeof(existGuard));
            found = true;
            break;
        }
    }

    if (!found)
    {
        guard_log(GUARD_ERROR, "Guard record not found");
        throw InvalidEntityPath("Guard record not found");
    }
}

void clear(const EntityPath& entityPath)
{
    auto path = entityPath;
    invalidateRecord(path);
}

void clear(const uint32_t recordId)
{
    auto id = recordId;
    invalidateRecord(id);
}

void clearAll()
{
    GuardFile file(guardFilePath);

    file.erase(0, file.size());
}

void invalidateAll()
{
    int pos = 0;
    GuardRecord existGuard;
    uint32_t offset = 0;

    memset(&existGuard, 0, sizeof(existGuard));
    GuardFile file(guardFilePath);
    file.read(0 + headerSize, &existGuard, sizeof(existGuard));
    if (isBlankRecord(existGuard))
    {
        guard_log(GUARD_INFO, "No GUARD records to clear");
    }
    else
    {
        for_each_guard(file, pos, existGuard)
        {
            offset = pos * sizeof(existGuard);
            existGuard.recordId = GUARD_RESOLVED;
            file.write(offset + headerSize, &existGuard, sizeof(existGuard));
        }
    }
}

void libguard_init(bool enableDevtree)
{
    initialize();
    guard_log(GUARD_DEBUG, "Device tree is set to %d", enableDevtree);
#ifdef DEV_TREE

    if (enableDevtree)
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
