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
 * @return void
 */
void init_libguard();

namespace utest
{
void setGuardFile(const fs::path& file);
}
} // namespace guard
} // namespace openpower
