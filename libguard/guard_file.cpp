// SPDX-License-Identifier: Apache-2.0

#include "guard_file.hpp"

#include "guard_exception.hpp"
#include "guard_log.hpp"

#include <cstring>
#include <fstream>

namespace openpower
{
namespace guard
{
using namespace openpower::guard::log;
using namespace openpower::guard::exception;

GuardFile::GuardFile(const fs::path& file) : guardFile(file)
{
    try
    {
        std::ifstream file(guardFile, std::ios::in | std::ios::binary);
        if (!file.good())
        {
            guard_log(GUARD_ERROR,
                      "Failed to open the GUARD file during initailization");
            throw GuardFileOpenFailed(
                "Exception thrown as failed to open the guard file");
        }
        file.seekg(0, file.end);
        if (file.fail())
        {
            guard_log(GUARD_ERROR,
                      "Failed to move to last position in guard file");
            throw GuardFileSeekFailed(
                "Exception thrown as failed to move to the "
                "last position in the file");
        }
        fileSize = file.tellg();
    }
    catch (openpower::guard::exception::GuardFileOpenFailed& ex)
    {
        throw GuardFileOpenFailed(ex.what());
    }
    catch (openpower::guard::exception::GuardFileSeekFailed& ex)
    {
        throw GuardFileSeekFailed(ex.what());
    }
}

void GuardFile::read(const uint64_t pos, void* dst, const uint64_t len)
{
    std::ifstream file(guardFile, std::ios::in | std::ios::binary);
    if (!file.good())
    {
        guard_log(GUARD_ERROR,
                  "Unable to open the guard file during read operation.");
        throw GuardFileOpenFailed(
            "Failed to open guard file in read function.");
    }
    file.seekg(pos, file.beg);
    if (file.fail())
    {
        guard_log(GUARD_ERROR,
                  "Unable to move to the position in the guard file at"
                  " position= 0x%016llx",
                  pos);
        throw GuardFileSeekFailed(
            "Failed to move"
            "to the position during read operation in the guard file");
    }
    file.read(reinterpret_cast<char*>(dst), len);
    if (file.fail())
    {
        guard_log(GUARD_ERROR,
                  "Unable to read from guard file at position= 0x%016llx", pos);
        throw GuardFileReadFailed("Failed to read from guard file.");
    }
    return;
}

void GuardFile::write(const uint64_t pos, const void* src, const uint64_t len)
{
    std::fstream file(guardFile,
                      std::ios::in | std::ios::out | std::ios::binary);
    if (!file.good())
    {
        guard_log(
            GUARD_ERROR,
            "Unable to open guard file while perfoming the write operation");
        throw GuardFileOpenFailed("Failed to open guard file to write");
    }

    file.seekp(pos, file.beg);
    if (file.fail())
    {
        guard_log(GUARD_ERROR,
                  "Unable to move to the position in the guard file."
                  " Position= 0x%016llx",
                  pos);
        throw GuardFileSeekFailed("Failed to move to the position during write "
                                  "operation in the guard file.");
    }

    file.write(reinterpret_cast<const char*>(src), len);
    if (file.fail())
    {
        guard_log(GUARD_ERROR, "Unable to write the record to GUARD file.");
        throw GuardFileWriteFailed("Failed to write to the guard file.");
    }
    return;
}

void GuardFile::erase(const uint64_t pos, const uint64_t len)
{
    int rlen = 0;
    static char buf[4096];
    memset(buf, ~0, sizeof(buf));
    if (len <= 0)
    {
        guard_log(GUARD_ERROR, "Length passed is %d which is not valid", len);
        throw InvalidEntry("Not a valid length value");
    }

    while (len - rlen > 0)
    {
        write(pos + rlen, buf,
              len - rlen > sizeof(buf) ? sizeof(buf) : len - rlen);
        rlen += (len - rlen > sizeof(buf) ? sizeof(buf) : len - rlen);
    }
    return;
}

uint32_t GuardFile::size()
{
    return fileSize;
}
} // namespace guard
} // namespace openpower
