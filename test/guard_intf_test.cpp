// SPDX-License-Identifier: Apache-2.0
#include "libguard/guard_common.hpp"
#include "libguard/guard_entity.hpp"
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
    bool isRecordDeleted = true;
    for (int i = 0; i < (int)records.size(); i++)
    {
        openpower::guard::GuardRecord record = records.at(i);
        if (record.targetId == entityPath)
        {
            isRecordDeleted = false;
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
    std::string phyPath = "/sys-0";
    std::optional<openpower::guard::EntityPath> entityPath =
        openpower::guard::getEntityPath(phyPath);
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
    phyPath = "/sys-0/node-0/proc-1/eq-0/fc-0";
    entityPath = openpower::guard::getEntityPath(phyPath);
    openpower::guard::create(*entityPath);
    phyPath = "/sys-0/node-0/proc-1/eq-0";
    entityPath = openpower::guard::getEntityPath(phyPath);
    EXPECT_THROW({ openpower::guard::create(*entityPath); },
                 std::runtime_error);
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
                 std::runtime_error);
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
    EXPECT_EQ(records.size(), 0);
}

TEST_F(TestGuardRecord, DeleteWithNotExistentEntity)
{
    openpower::guard::libguard_init();
    std::string physPath{"/sys-0/node-0/proc-0/eq-0/fc-0/core-0"};
    std::optional<openpower::guard::EntityPath> entityPath =
        openpower::guard::getEntityPath(physPath);

    // Trying to delete entity which is not present
    EXPECT_THROW({ openpower::guard::clear(*entityPath); }, std::runtime_error);
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
    EXPECT_EQ(records.size(), 0);
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
                 std::runtime_error);
}
