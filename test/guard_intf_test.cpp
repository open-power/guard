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
        ASSERT_NE(nullptr, dirPtr);
        guardDir = dirPtr;
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

    using retVals = std::tuple<openpower::guard::GuardRecords,
                               std::optional<openpower::guard::EntityPath>>;
    retVals LibGuardInit(const std::string& phyPath)
    {
        openpower::guard::libguard_init();
        auto entityPath = openpower::guard::getEntityPath(phyPath);
        EXPECT_NE(entityPath, std::nullopt);
        openpower::guard::create(*entityPath);
        return std::make_tuple(openpower::guard::getAll(), entityPath);
    }

  protected:
    fs::path guardFile;
    std::string guardDir;
};

TEST_F(TestGuardRecord, CreateGuardRecord)
{
    //! TODO need to test serial number and part number once device tree support
    //! is available.
    auto [records, entityPath] =
        LibGuardInit("/sys-0/node-0/proc-1/eq-0/fc-0/core-0");
    EXPECT_EQ(records.size(), 1);
    openpower::guard::GuardRecord record = records.at(0);
    EXPECT_EQ(record.targetId, entityPath);
}

TEST_F(TestGuardRecord, ClearGuardGoodPathTest)
{
    LibGuardInit("/sys-0/node-0/proc-1/eq-0/fc-0/core-0");
    openpower::guard::clearAll();
    openpower::guard::GuardRecords records = openpower::guard::getAll();
    EXPECT_EQ(records.size(), 0);
}

TEST_F(TestGuardRecord, DeleteGuardGoodPathTest)
{
    LibGuardInit("/sys-0/node-0/proc-1/eq-0/fc-0/core-0");
    LibGuardInit("/sys-0/node-0/proc-1/eq-0/fc-0/core-1");
    LibGuardInit("/sys-0/node-0/proc-0/eq-0/fc-0/core-0");
    LibGuardInit("/sys-0/node-0/proc-0/eq-0/fc-0/core-1");

    auto entityPath = openpower::guard::getEntityPath(
        "/sys-0/node-0/proc-0/eq-0/fc-0/core-0");
    openpower::guard::clear(*entityPath);
    auto records = openpower::guard::getAll();
    auto isRecordDeleted = false;
    for (const auto& record : records)
    {
        if (record.targetId == entityPath)
        {
            EXPECT_EQ(record.recordId, GUARD_RESOLVED);
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
    LibGuardInit("/sys-0/node-0/proc-1/eq-0/fc-0/core-0");
    LibGuardInit("/sys-0/node-0/proc-1/eq-0/fc-0/core-1");
    LibGuardInit("/sys-0/node-0/proc-0/eq-0/fc-0/core-0");
    LibGuardInit("/sys-0/node-0/proc-0/eq-0/fc-0/core-1");
    LibGuardInit("/sys-0/node-0/proc-2/eq-0/fc-0/core-0");
    std::string phyPath = "/sys-0/node-0/dimm-1";
    auto entityPath = openpower::guard::getEntityPath(phyPath);
    EXPECT_THROW(openpower::guard::create(*entityPath),
                 openpower::guard::exception::GuardFileOverFlowed);
}

TEST_F(TestGuardRecord, AlreadyGuardedTC)
{
    auto entityPath =
        std::get<1>(LibGuardInit("/sys-0/node-0/proc-0/eq-0/fc-0/core-0"));
    // Trying to guard again with same entity
    EXPECT_THROW(openpower::guard::create(*entityPath),
                 openpower::guard::exception::AlreadyGuarded);
}

TEST_F(TestGuardRecord, GetCreatedGuardRecordTC)
{
    auto [records, entityPath] =
        LibGuardInit("/sys-0/node-0/proc-0/eq-0/fc-0/core-0");

    // Validate the return guard record
    // Note: Only one record created in this test case and also
    // by default created guard will be considered as Manual
    // so expectation will be like below.
    EXPECT_EQ(records.size(), 1);
    EXPECT_EQ(records[0].recordId, 1);
    EXPECT_EQ(records[0].targetId, entityPath);
    EXPECT_EQ(records[0].elogId, 0);
    EXPECT_EQ(records[0].errType, openpower::guard::GardType::GARD_User_Manual);
}

TEST_F(TestGuardRecord, DeleteByEntityPath)
{
    auto [records, entityPath] =
        LibGuardInit("/sys-0/node-0/proc-0/eq-0/fc-0/core-0");
    EXPECT_EQ(records.size(), 1);
    EXPECT_NE(GUARD_RESOLVED, records[0].recordId);
    // Trying to delete
    openpower::guard::clear(*entityPath);

    // Make sure is deleted
    records.clear();
    records = openpower::guard::getAll();
    EXPECT_EQ(records.size(), 1);
    EXPECT_EQ(GUARD_RESOLVED, records[0].recordId);
}

TEST_F(TestGuardRecord, DeleteWithNotExistentEntity)
{
    auto entityPath =
        std::get<1>(LibGuardInit("/sys-0/node-0/proc-0/eq-0/fc-0/core-0"));

    std::string phyPath = "/sys-0/node-0/proc-0/eq-0/fc-0/core-1";
    entityPath = openpower::guard::getEntityPath(phyPath);
    // Trying to delete entity which is not present
    EXPECT_THROW(openpower::guard::clear(*entityPath),
                 openpower::guard::exception::InvalidEntityPath);
}

TEST_F(TestGuardRecord, DeleteByRecordId)
{
    auto records =
        std::get<0>(LibGuardInit("/sys-0/node-0/proc-0/eq-0/fc-0/core-0"));
    EXPECT_EQ(records.size(), 1);

    // Trying to delete with returned record id
    openpower::guard::clear(records[0].recordId);

    // Make sure is deleted
    records = openpower::guard::getAll();
    EXPECT_EQ(records.size(), 1);
    EXPECT_EQ(records[0].recordId, GUARD_RESOLVED);
}

TEST_F(TestGuardRecord, DeleteWithNotExistentRecordId)
{
    auto records =
        std::get<0>(LibGuardInit("/sys-0/node-0/proc-0/eq-0/fc-0/core-0"));
    EXPECT_EQ(records.size(), 1);

    // Trying to delete a record by using returned record id with increment
    EXPECT_THROW(openpower::guard::clear(records[0].recordId + 1),
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
    LibGuardInit("/sys-0/node-0/proc-0/eq-0/fc-0/core-0");
    LibGuardInit("/sys-0/node-0/proc-0/eq-0/fc-0/core-1");
    openpower::guard::invalidateAll();
    auto records = openpower::guard::getAll();
    EXPECT_EQ(records.size(), 2);
    auto record = records.at(0);
    EXPECT_EQ(record.recordId, GUARD_RESOLVED);
    record = records.at(1);
    EXPECT_EQ(record.recordId, GUARD_RESOLVED);
}

TEST_F(TestGuardRecord, ClearResolvedGuardRecord)
{
    LibGuardInit("/sys-0/node-0/dimm-0");
    LibGuardInit("/sys-0/node-0/dimm-1");
    LibGuardInit("/sys-0/node-0/dimm-2");
    auto [records, entityPath] = LibGuardInit("/sys-0/node-0/dimm-3");
    EXPECT_EQ(records.size(), 4);
    openpower::guard::clear(*entityPath);
    records = openpower::guard::getAll();
    EXPECT_EQ(records.size(), 4);
    EXPECT_EQ(GUARD_RESOLVED, records[3].recordId);
    records = std::get<0>(LibGuardInit("/sys-0/node-0/dimm-4"));
    EXPECT_EQ(records.size(), 5);
    EXPECT_EQ(GUARD_RESOLVED, records[3].recordId);
    EXPECT_NE(GUARD_RESOLVED, records[4].recordId);
}
