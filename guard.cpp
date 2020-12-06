// SPDX-License-Identifier: Apache-2.0
#include "libguard/guard_interface.hpp"
#include "libguard/include/guard_record.hpp"

#include <CLI/CLI.hpp>

using namespace std;
using namespace openpower::guard;

void guardList()
{
    auto records = getAll();
    if (!records.size())
    {
        std::cout << "No Records to display" << std::endl;
        return;
    }
    std::cout << "ID       | ERROR    |  Type  | Path " << std::endl;
    for (const auto& elem : records)
    {
        if (elem.errType != GARD_Reconfig)
        {
            std::cout << std::hex << std::setw(8) << std::setfill('0')
                      << elem.recordId;
            std::cout << " | ";
            std::cout << std::hex << std::setw(8) << std::setfill('0')
                      << elem.elogId;

            std::cout << " | ";
            std::optional<std::string> gReasonToStr =
                guardReasonToStr(elem.errType);
            std::cout << *gReasonToStr;
            std::cout << " | ";
            std::optional<std::string> physicalPath =
                getPhysicalPath(elem.targetId);
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
    }
}

void guardDelete(const std::string& physicalPath)
{
    std::optional<EntityPath> entityPath = getEntityPath(physicalPath);
    if (!entityPath)
    {
        std::cerr << "Unsupported physical path " << physicalPath << std::endl;
        return;
    }
    clear(*entityPath);
}

void guardClear()
{
    clearAll();
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
        CLI::App app{"GUARD Tool"};
        std::optional<std::string> createGuardStr;
        std::optional<std::string> deleteGuardStr;
        bool listGuardRecords = false;
        bool clearAll = false;
        bool gversion = false;

        app.set_help_flag("-h, --help",
                          "Use the below listed functions.\n"
                          "Warning: Don't try guard on non guardable units "
                          "(sys, perv)");
        app.add_option("-c, --create", createGuardStr,
                       "Create GUARD record, expects physical path as input");
        app.add_option("-d, --delete", deleteGuardStr,
                       "Delete GUARD record, expects physical path as input");
        app.add_flag("-l, --list", listGuardRecords,
                     "Listing of GUARDed resources.");
        app.add_flag("-r, --clearall", clearAll,
                     "Clears GUARD states for all resources");
        app.add_flag("-v, --version", gversion, "Version of GUARD tool");

        CLI11_PARSE(app, argc, argv);

        libguard_init();

        if (createGuardStr)
        {
            guardCreate(*createGuardStr);
        }
        else if (deleteGuardStr)
        {
            guardDelete(*deleteGuardStr);
        }
        else if (clearAll)
        {
            guardClear();
        }
        else if (listGuardRecords)
        {
            guardList();
        }
        else if (gversion)
        {
            std::cout << "GUARD Tool version is " << GUARD_VERSION << std::endl;
        }
        else
        {
            exitWithError(app.help("", CLI::AppFormatMode::All),
                          "Invalid option");
        }
    }
    catch (const std::exception& ex)
    {
        std::cout << "Exception " << ex.what() << std::endl;
    }
    return 0;
}
