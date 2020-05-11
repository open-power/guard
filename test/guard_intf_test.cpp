// SPDX-License-Identifier: Apache-2.0
#include "libguard/guard_file.hpp"
#include "libguard/guard_interface.hpp"
#include "libguard/include/guard_record.hpp"

#include <filesystem>
#include <fstream>

#include <gtest/gtest.h>

namespace fs = std::filesystem;

class TestGuardRecord : public ::testing::Test
{
  public:
    TestGuardRecord()
    {
    }

    void SetUp() override
    {
        char dirTemplate[] = "/tmp/FakeGuard.XXXXXX";
        auto dirPtr = mkdtemp(dirTemplate);
        if (dirPtr == NULL)
        {
            throw std::bad_alloc();
        }
        guardDir = std::string(dirPtr);
        fs::create_directories(guardDir);
        guardFile = guardDir;
        guardFile /= "GUARD";

        //! create a guard file
        std::ofstream file(guardFile, std::ios::out | std::ios::binary);
        static char buf[8192];
        memset(buf, ~0, sizeof(buf));
        file.write(reinterpret_cast<const char*>(buf), 8192);
        uint8_t magicNumber[8] = {'G', 'U', 'A', 'R', 'D', 'R', 'E', 'C'};
        file.seekp(0, file.beg);
        file.write(reinterpret_cast<const char*>(magicNumber), 8);
        uint8_t version_padding[8];
        memset(&version_padding, 0, 8);
        file.write(reinterpret_cast<const char*>(version_padding), 8);

        //! set guard file to use
        openpower::guard::utest::setGuardFile(guardFile);
    }
    void TearDown() override
    {
        fs::remove_all(guardDir);
    }

  protected:
    fs::path guardFile;
    std::string guardDir;
};

TEST_F(TestGuardRecord, CreateGuardRecord)
{
    //! TODO need to test serial number and part number once device tree support
    //! is available.
    openpower::guard::EntityPath entityPath = {
        0x26, {1, 0, 2, 0, 5, 1, 35, 0, 83, 0, 7, 0}};
    openpower::guard::create(entityPath);
    openpower::guard::GuardRecords records = openpower::guard::getAll();
    EXPECT_EQ(records.size(), 1);
    openpower::guard::GuardRecord record = records.at(0);
    EXPECT_EQ(record.targetId, entityPath);
}

TEST_F(TestGuardRecord, ClearGuardGoodPathTest)
{
    openpower::guard::EntityPath entityPath = {
        0x26, {1, 0, 2, 0, 5, 1, 35, 0, 83, 0, 7, 0}};
    openpower::guard::create(entityPath);
    openpower::guard::clearAll();
    openpower::guard::GuardRecords records = openpower::guard::getAll();
    EXPECT_EQ(records.size(), 0);
}

TEST_F(TestGuardRecord, DeleteGuardGoodPathTest)
{
    openpower::guard::EntityPath entityPath = {
        0x26, {1, 0, 2, 0, 5, 1, 35, 0, 83, 0, 7, 0}};
    openpower::guard::create(entityPath);
    entityPath = {0x26, {1, 0, 2, 0, 5, 0, 35, 0, 83, 0, 7, 1}};
    openpower::guard::create(entityPath);
    entityPath = {0x26, {1, 0, 2, 0, 5, 1, 35, 0, 83, 0, 7, 1}};
    openpower::guard::create(entityPath);
    openpower::guard::EntityPath entityPathTodel = {
        0x26, {1, 0, 2, 0, 5, 0, 35, 0, 83, 0, 7, 1}};
    openpower::guard::clear(entityPathTodel);
    openpower::guard::GuardRecords records = openpower::guard::getAll();
    bool isRecordDeleted = true;
    for (int i = 0; i < (int)records.size(); i++)
    {
        openpower::guard::GuardRecord record = records.at(i);
        if (record.targetId == entityPathTodel)
        {
            isRecordDeleted = false;
            break;
        }
    }
    EXPECT_EQ(isRecordDeleted, true);
}
