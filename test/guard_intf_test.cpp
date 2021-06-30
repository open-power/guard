// SPDX-License-Identifier: Apache-2.0
#include "libguard/guard_common.hpp"
#include "libguard/guard_entity.hpp"
#include "libguard/guard_exception.hpp"
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
        static char buf[656];
        memset(buf, ~0, sizeof(buf));
        file.write(reinterpret_cast<const char*>(buf), 656);
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
    openpower::guard::libguard_init();
    std::string phyPath = "/sys-0/node-0/proc-1/eq-0/fc-0/core-0";
    std::optional<openpower::guard::EntityPath> entityPath =
        openpower::guard::getEntityPath(phyPath);
    EXPECT_NE(entityPath, std::nullopt);
    openpower::guard::create(*entityPath);
    openpower::guard::GuardRecords records = openpower::guard::getAll();
    EXPECT_EQ(records.size(), 1);
    openpower::guard::GuardRecord record = records.at(0);
    EXPECT_EQ(record.targetId, entityPath);
}

TEST_F(TestGuardRecord, ClearGuardGoodPathTest)
{
    openpower::guard::libguard_init();
    std::string phyPath = "/sys-0/node-0/proc-1/eq-0/fc-0/core-0";
    std::optional<openpower::guard::EntityPath> entityPath =
        openpower::guard::getEntityPath(phyPath);
    EXPECT_NE(entityPath, std::nullopt);
    openpower::guard::create(*entityPath);
    openpower::guard::clearAll();
    openpower::guard::GuardRecords records = openpower::guard::getAll();
    EXPECT_EQ(records.size(), 0);
}

TEST_F(TestGuardRecord, DeleteGuardGoodPathTest)
{
    openpower::guard::libguard_init();
    std::string phyPath = "/sys-0/node-0/proc-1/eq-0/fc-0/core-0";
    std::optional<openpower::guard::EntityPath> entityPath =
        openpower::guard::getEntityPath(phyPath);
    openpower::guard::create(*entityPath);
    phyPath = "/sys-0/node-0/proc-1/eq-0/fc-0/core-1";
    entityPath = openpower::guard::getEntityPath(phyPath);
    openpower::guard::create(*entityPath);
    phyPath = "/sys-0/node-0/proc-0/eq-0/fc-0/core-0";
    entityPath = openpower::guard::getEntityPath(phyPath);
    openpower::guard::create(*entityPath);
    phyPath = "/sys-0/node-0/proc-0/eq-0/fc-0/core-1";
    entityPath = openpower::guard::getEntityPath(phyPath);
    openpower::guard::create(*entityPath);

    phyPath = "/sys-0/node-0/proc-0/eq-0/fc-0/core-0";
    entityPath = openpower::guard::getEntityPath(phyPath);
    openpower::guard::clear(*entityPath);
    openpower::guard::GuardRecords records = openpower::guard::getAll();
    bool isRecordDeleted = false;
    for (int i = 0; i < (int)records.size(); i++)
    {
        openpower::guard::GuardRecord record = records.at(i);
        if (record.targetId == entityPath)
        {
            EXPECT_EQ(record.recordId, 0xFFFFFFFF);
            isRecordDeleted = true;
            break;
        }
    }
    EXPECT_EQ(isRecordDeleted, true);
}

TEST_F(TestGuardRecord, NegTestCaseEP)
{
    openpower::guard::libguard_init();
    std::string phyPath = "/sys-0/node-0";
    std::optional<openpower::guard::EntityPath> entityPath =
        openpower::guard::getEntityPath(phyPath);
    EXPECT_EQ(entityPath, std::nullopt);
}

TEST_F(TestGuardRecord, NegTestCaseFullGuardFile)
{
    openpower::guard::libguard_init();
    std::string phyPath = " ";
    std::optional<openpower::guard::EntityPath> entityPath;
    phyPath = "/sys-0/node-0/proc-1/eq-0/fc-0/core-1";
    entityPath = openpower::guard::getEntityPath(phyPath);
    openpower::guard::create(*entityPath);
    phyPath = "/sys-0/node-0/proc-0/eq-0/fc-0/core-0";
    entityPath = openpower::guard::getEntityPath(phyPath);
    openpower::guard::create(*entityPath);
    phyPath = "/sys-0/node-0/proc-0/eq-0/fc-0/core-1";
    entityPath = openpower::guard::getEntityPath(phyPath);
    openpower::guard::create(*entityPath);
    phyPath = "/sys-0/node-0/proc-1/eq-0/fc-0/core-0";
    entityPath = openpower::guard::getEntityPath(phyPath);
    openpower::guard::create(*entityPath);
    phyPath = "/sys-0/node-0/dimm-1";
    entityPath = openpower::guard::getEntityPath(phyPath);
    EXPECT_THROW({ openpower::guard::create(*entityPath); },
                 openpower::guard::exception::GuardFileOverFlowed);
}

TEST_F(TestGuardRecord, AlreadyGuardedTC)
{
    openpower::guard::libguard_init();
    std::string physPath{"/sys-0/node-0/proc-0/eq-0/fc-0/core-0"};
    std::optional<openpower::guard::EntityPath> entityPath =
        openpower::guard::getEntityPath(physPath);
    openpower::guard::create(*entityPath);

    // Trying to guard again with same entity
    EXPECT_THROW({ openpower::guard::create(*entityPath); },
                 openpower::guard::exception::AlreadyGuarded);
}

TEST_F(TestGuardRecord, GetCreatedGuardRecordTC)
{
    openpower::guard::libguard_init();
    std::string physPath{"/sys-0/node-0/proc-0/eq-0/fc-0/core-0"};
    std::optional<openpower::guard::EntityPath> entityPath =
        openpower::guard::getEntityPath(physPath);

    openpower::guard::GuardRecord retGuardRecord =
        openpower::guard::create(*entityPath);

    // Validate the return guard record
    // Note: Only one record created in this test case and also
    // by default created guard will be considered as Manual
    // so expectation will be like below.
    EXPECT_EQ(retGuardRecord.recordId, 1);
    EXPECT_EQ(retGuardRecord.targetId, entityPath);
    EXPECT_EQ(retGuardRecord.elogId, 0);
    EXPECT_EQ(retGuardRecord.errType,
              openpower::guard::GardType::GARD_User_Manual);
}

TEST_F(TestGuardRecord, DeleteByEntityPath)
{
    openpower::guard::libguard_init();
    std::string physPath{"/sys-0/node-0/proc-0/eq-0/fc-0/core-0"};
    std::optional<openpower::guard::EntityPath> entityPath =
        openpower::guard::getEntityPath(physPath);

    openpower::guard::create(*entityPath);

    // Trying to delete
    openpower::guard::clear(*entityPath);

    // Make sure is deleted
    openpower::guard::GuardRecords records = openpower::guard::getAll();
    EXPECT_EQ(records.size(), 1);
}

TEST_F(TestGuardRecord, DeleteWithNotExistentEntity)
{
    openpower::guard::libguard_init();
    std::string physPath{"/sys-0/node-0/proc-0/eq-0/fc-0/core-0"};
    std::optional<openpower::guard::EntityPath> entityPath =
        openpower::guard::getEntityPath(physPath);

    // Trying to delete entity which is not present
    EXPECT_THROW({ openpower::guard::clear(*entityPath); },
                 openpower::guard::exception::InvalidEntityPath);
}

TEST_F(TestGuardRecord, DeleteByRecordId)
{
    openpower::guard::libguard_init();
    std::string physPath{"/sys-0/node-0/proc-0/eq-0/fc-0/core-0"};
    std::optional<openpower::guard::EntityPath> entityPath =
        openpower::guard::getEntityPath(physPath);

    openpower::guard::GuardRecord retGuardRecord =
        openpower::guard::create(*entityPath);

    // Trying to delete with returned record id
    openpower::guard::clear(retGuardRecord.recordId);

    // Make sure is deleted
    openpower::guard::GuardRecords records = openpower::guard::getAll();
    EXPECT_EQ(records.size(), 1);
    openpower::guard::GuardRecord record = records.at(0);
    EXPECT_EQ(record.recordId, 0xFFFFFFFF);
}

TEST_F(TestGuardRecord, DeleteWithNotExistentRecordId)
{
    openpower::guard::libguard_init();
    std::string physPath{"/sys-0/node-0/proc-0/eq-0/fc-0/core-0"};
    std::optional<openpower::guard::EntityPath> entityPath =
        openpower::guard::getEntityPath(physPath);

    openpower::guard::GuardRecord retGuardRecord =
        openpower::guard::create(*entityPath);

    // Trying to delete a record by using returned record id with increment
    EXPECT_THROW({ openpower::guard::clear(retGuardRecord.recordId + 1); },
                 openpower::guard::exception::InvalidEntityPath);
}

TEST_F(TestGuardRecord, GetGuardFilePathTC)
{
    openpower::guard::libguard_init();

    std::string retGuardFilePath = openpower::guard::getGuardFilePath();

    // Make sure the guard file matched with TC setup guardFile.
    EXPECT_EQ(retGuardFilePath, guardFile);
}

TEST_F(TestGuardRecord, GetGuardFilePathWhenLibguradDidNotInitTC)
{
    // Unset the guard file for this UT alone
    openpower::guard::utest::setGuardFile("");

    // Checking without libguard_init() call.
    EXPECT_THROW({ openpower::guard::getGuardFilePath(); },
                 openpower::guard::exception::GuardFileOpenFailed);

    // Set the guard file since UT reached the end
    openpower::guard::utest::setGuardFile(guardFile);

    // Make sure the guard file is set since this UT should not break
    // the other UTs because SetUp() is common to all UTs which are drived from
    // TestGuardRecord class
    std::string retGuardFilePath = openpower::guard::getGuardFilePath();
    EXPECT_EQ(retGuardFilePath, guardFile);
}

TEST_F(TestGuardRecord, ClearGuardInvalidateAllPathTest)
{
    openpower::guard::libguard_init();
    std::string phyPath = "/sys-0/node-0/proc-1/eq-0/fc-0/core-0";
    std::optional<openpower::guard::EntityPath> entityPath =
        openpower::guard::getEntityPath(phyPath);
    EXPECT_NE(entityPath, std::nullopt);
    openpower::guard::create(*entityPath);
    phyPath = "/sys-0/node-0/proc-1/eq-0/fc-0/core-1";
    entityPath = openpower::guard::getEntityPath(phyPath);
    openpower::guard::create(*entityPath);
    openpower::guard::invalidateAll();
    openpower::guard::GuardRecords records = openpower::guard::getAll();
    EXPECT_EQ(records.size(), 2);
    openpower::guard::GuardRecord record = records.at(0);
    EXPECT_EQ(record.recordId, 0xFFFFFFFF);
    record = records.at(1);
    EXPECT_EQ(record.recordId, 0xFFFFFFFF);
}

TEST_F(TestGuardRecord, ClearResolvedGuardRecord)
{
    openpower::guard::libguard_init();
    std::string phyPath = "/sys-0/node-0/dimm-0";
    std::optional<openpower::guard::EntityPath> entityPath =
        openpower::guard::getEntityPath(phyPath);
    openpower::guard::create(*entityPath);
    phyPath = "/sys-0/node-0/dimm-1";
    entityPath = openpower::guard::getEntityPath(phyPath);
    openpower::guard::create(*entityPath);
    phyPath = "/sys-0/node-0/dimm-2";
    entityPath = openpower::guard::getEntityPath(phyPath);
    openpower::guard::create(*entityPath);
    phyPath = "/sys-0/node-0/dimm-3";
    entityPath = openpower::guard::getEntityPath(phyPath);
    openpower::guard::create(*entityPath);
    openpower::guard::clear(*entityPath);
    phyPath = "/sys-0/node-0/dimm-4";
    entityPath = openpower::guard::getEntityPath(phyPath);
    openpower::guard::create(*entityPath);
    openpower::guard::GuardRecords records = openpower::guard::getAll();
    EXPECT_EQ(records.size(), 4);
}
