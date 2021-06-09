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
 *         Throw exception on failure
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
 */
GuardRecords getAll();

/**
 * @brief Clear the guard record
 *
 * @param[in] entityPath entity path
 * @return NULL on success
 *         Throw exception on failure
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
 *         Throw exception on failure.
 *
 */
void clear(const uint32_t recordId);

/**
 * @brief Clear all the guard records
 *
 * @return NULL
 */
void clearAll();

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

namespace utest
{
void setGuardFile(const fs::path& file);
}
} // namespace guard
} // namespace openpower
