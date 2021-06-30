// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "guard_entity.hpp"
#include "include/guard_record.hpp"

#include <filesystem>

namespace openpower
{
namespace guard
{
namespace fs = std::filesystem;
/**
 * @brief Create a guard record on the PNOR Partition file
 *
 * @param[in] entityPath entity path of the guarded record
 * @param[in] eId errorlog ID
 * @param[in] eType errorlog type
 * @return created guard record in host endianess format on success
 *         Throw following exceptions on failure:
 *         -GuardFileOverFlowed
 *         -AlreadyGuarded
 *         -GuardFileOpenFailed
 *         -GuardFileSeekFailed
 *         -GuardFileReadFailed
 *         -GuardFileWriteFailed
 *
 * @note EntityPath provided conversion constructor so, same api can use to pass
 * array of uint8_t buffer and conversion constructor automatically will take
 * care conversion from raw data to EntityPath.
 */
GuardRecord create(const EntityPath& entityPath, uint32_t eId = 0,
                   uint8_t eType = GARD_User_Manual);

/**
 * @brief Get all the guard records
 *
 * @return GuardRecords List of Guard Records.
 *         On failure will throw below exceptions:
 *         -GuardFileOpenFailed
 *         -GuardFileSeekFailed
 *         -GuardFileReadFailed
 */
GuardRecords getAll();

/**
 * @brief Clear the guard record
 *
 * @param[in] entityPath entity path
 * @return NULL on success
 *         Throw following exceptions on failure:
 *         -InvalidEntry
 *         -InvalidEntityPath
 *         -GuardFileOpenFailed
 *         -GuardFileSeekFailed
 *         -GuardFileReadFailed
 *         -GuardFileWriteFailed
 *
 * @note EntityPath provided conversion constructor so, same api can use to pass
 * array of uint8_t buffer and conversion constructor automatically will take
 * care conversion from raw data to EntityPath.
 */
void clear(const EntityPath& entityPath);

/**
 * @brief Clear the guard record based on given record id
 *
 * @return NULL on success.
 *         Throw following exceptions on failure:
 *         -InvalidEntry
 *         -InvalidEntityPath
 *         -GuardFileOpenFailed
 *         -GuardFileSeekFailed
 *         -GuardFileWriteFailed
 *         -GuardFileReadFailed
 *
 */
void clear(const uint32_t recordId);

/**
 * @brief Clear all the guard records
 *
 * @return NULL on success
 *         Throw below exceptions on failure:
 *         -GuardFileOpenFailed
 *         -GuardFileSeekFailed
 *         -GuardFileWriteFailed
 */
void clearAll();

/**
 * @brief Invalidates all the guard records
 * i.e. mark recordId as 0xFFFFFFFF.
 *
 * @return NULL on success
 *         Throw throw below exceptions on failure:
 *         -GuardFileOpenFailed
 *         -GuardFileSeekFailed
 *         -GuardFileWriteFailed
 *         -GuardFileReadFailed
 */
void invalidateAll();

/**
 * @brief To initialize libguard
 *
 * This function is used to get guard file which is used to maintain
 * faulty replaceable units (FRUs) and also used to initialize power
 * system device tree if required.
 *
 * @param[in] enableDevtree used to decide if libguard need to
 *            initialize device tree or not, default is true.
 * @return void
 *
 * @note device tree initialization should happen once.
 */
void libguard_init(bool enableDevtree = true);

/**
 * @brief Used to get guard file path which is using by libguard.
 *
 * @return guard file path on success
 *         Throws GuardFileOpenFailed exception if guard file is not
 * initialised.
 *
 * @note This function should call after libguard_init()
 */
const fs::path& getGuardFilePath();

namespace utest
{
void setGuardFile(const fs::path& file);
}
} // namespace guard
} // namespace openpower
