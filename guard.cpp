// SPDX-License-Identifier: Apache-2.0
#include "libguard/guard_interface.hpp"
#include "libguard/include/guard_record.hpp"

#include <config.h>

#include <CLI/CLI.hpp>

using namespace std;
using namespace openpower::guard;

void printHeader()
{
    std::cout << "ID       | ERROR    |  Type  | Path " << std::endl;
}

void printRecord(const GuardRecord& record)
{
    std::cout << "0x" << std::hex << std::setw(8) << std::setfill('0')
              << record.recordId;

    std::cout << " | ";
    std::cout << "0x" << std::hex << std::setw(8) << std::setfill('0')
              << record.elogId;

    std::cout << " | ";
    std::cout << std::left << std::setfill(' ') << std::setw(15)
              << guardReasonToStr(record.errType);

    std::cout << " | ";
    std::optional<std::string> physicalPath = getPhysicalPath(record.targetId);
    if (!physicalPath)
    {
        std::cout << "Unknown ";
    }
    else
    {
        std::cout << *physicalPath;
    }
    std::cout << std::endl;
}

void guardList(bool displayResolved)
{
    // Don't get ephemeral records because those type records are not intended
    // to expose to the end user, just created for internal purpose to use
    // by the BMC and Hostboot.
    auto records = getAll(true);
    if (!records.size())
    {
        std::cout << "No Records to display" << std::endl;
        return;
    }

    bool isHeaderPrinted = false;
    for (const auto& elem : records)
    {
        // To list resolved records as user wants to list resolved records
        if (displayResolved && (elem.recordId != GUARD_RESOLVED))
        {
            continue;
        }
        // To list unresolved records as user wants to list unresolved records
        else if (!displayResolved && (elem.recordId == GUARD_RESOLVED))
        {
            continue;
        }

        // Don't print the header if already printed header since
        // records mixed of resolved and unresolved records so if have
        // only either one in retrieved records and user tried to see opposite
        // one then we should not print header else user will get confused.
        if (!isHeaderPrinted)
        {
            printHeader();
            isHeaderPrinted = true;
        }
        printRecord(elem);
    }

    if (!isHeaderPrinted)
    {
        std::cout << "No "
                  << (displayResolved == true ? "resolved" : "unresolved")
                  << " records to display" << std::endl;
    }
}

/**
 * @brief This function is used to list out the ephemeral type records.
 *
 * @return NULL
 *
 * @note This function will list out both (unresolved and resolved) records.
 */
void guardListEphemeralRecords()
{
    auto records = getAll();
    if (!records.size())
    {
        std::cout << "No Records to display" << std::endl;
        return;
    }

    bool isHeaderPrinted = false;
    for (const auto& record : records)
    {
        if (!isEphemeralType(record.errType))
        {
            continue;
        }

        if (!isHeaderPrinted)
        {
            printHeader();
            isHeaderPrinted = true;
        }
        printRecord(record);
    }

    if (!isHeaderPrinted)
    {
        std::cout << "No ephemeral records to display" << std::endl;
    }
}

void guardDelete(const uint32_t recordId)
{
    clear(recordId);
}

void guardClear()
{
    clearAll();
}

void guardInvalidateAll()
{
    invalidateAll();
}

void guardCreate(const std::string& physicalPath)
{
    std::optional<EntityPath> entityPath = getEntityPath(physicalPath);
    if (!entityPath)
    {
        std::cerr << "Unsupported physical path " << physicalPath << std::endl;
        return;
    }
    create(*entityPath);
    std::cout << "Success" << std::endl;
}

static void exitWithError(const std::string& help, const char* err)
{
    std::cerr << "ERROR: " << err << std::endl << help << std::endl;
    exit(-1);
}

int main(int argc, char** argv)
{
    try
    {
        CLI::App app{"Guard Tool"};
        std::optional<std::string> createGuardStr;
        std::optional<uint32_t> recordId;
        bool listGuardRecords = false;
        bool clearAll = false;
        bool listResolvedGuardRecords = false;
        bool listEphemeralRecords = false;
        bool invalidateAll = false;
        bool gversion = false;

        app.set_help_flag("-h, --help", "Guard CLI tool options");
        app.add_option("-c, --create", createGuardStr,
                       "Create Guard record, expects physical path as input");
        app.add_option(
            "-i, --invalidate", recordId,
            "Invalidate a single Guard record, expects record id as input");
        app.add_flag("-I, --invalidate-all", invalidateAll,
                     "Invalidates all the Guard records");
        app.add_flag("-l, --list", listGuardRecords,
                     "List all the Guard'ed resources");
        app.add_flag("-r, --reset", clearAll, "Erase all the Guard records");
        app.add_flag("-a, --listresolvedrecords", listResolvedGuardRecords,
                     "List all the resolved Guard'ed resources")
            ->group("");
        app.add_flag("-e, --listEphemeralRecords", listEphemeralRecords,
                     "List all the resolved and unresolved ephemeral Guard'ed "
                     "resources")
            ->group("");
        app.add_flag("-v, --version", gversion, "Version of GUARD tool");

        CLI11_PARSE(app, argc, argv);

        libguard_init();

        if (createGuardStr)
        {
            guardCreate(*createGuardStr);
        }
        else if (recordId)
        {
            guardDelete(*recordId);
        }
        else if (clearAll)
        {
            guardClear();
        }
        else if (listGuardRecords)
        {
            guardList(false);
        }
        else if (listResolvedGuardRecords)
        {
            guardList(true);
        }
        else if (listEphemeralRecords)
        {
            guardListEphemeralRecords();
        }
        else if (invalidateAll)
        {
            guardInvalidateAll();
        }
        else if (gversion)
        {
            std::cout << "Guard tool " << GUARD_VERSION << std::endl;
        }
        else
        {
            exitWithError(app.help("", CLI::AppFormatMode::All),
                          "Invalid option");
        }
    }
    catch (const std::exception& ex)
    {
        std::cout << "Exception: " << ex.what() << std::endl;
    }
    return 0;
}
