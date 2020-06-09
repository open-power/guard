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
 * @return NULL
 *
 * @note EntityPath provided conversion constructor so, same api can use to pass
 * array of uint8_t buffer and conversion constructor automatically will take
 * care conversion from raw data to EntityPath.
 */
void create(const EntityPath& entityPath, uint32_t eId = 0,
            uint8_t eType = 0xD2);

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
 * @return NULL
 *
 * @note EntityPath provided conversion constructor so, same api can use to pass
 * array of uint8_t buffer and conversion constructor automatically will take
 * care conversion from raw data to EntityPath.
 */
void clear(const EntityPath& entityPath);

/**
 * @brief Clear all the guard records
 *
 * @return NULL
 */
void clearAll();

/**
 * @brief To init libguard library and its depends library if required
 *
 * @param[in] guardInit default true incase device tree is already
 *            initialized need to set false.
 * @return void
 */
void libguard_init(bool guardInit = true);

namespace utest
{
void setGuardFile(const fs::path& file);
}
} // namespace guard
} // namespace openpower
