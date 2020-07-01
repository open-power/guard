// SPDX-License-Identifier: Apache-2.0
#ifndef __PHAL_DEVTREE_H
#define __PHAL_DEVTREE_H

/**
 * @file phal_devtree.hpp
 * @brief Used to get physical path value from power system device tree
 *
 * The power specific system device tree will contains all the FRU (target)
 * details. So, To guard the faulted FRU's, guard application need to get
 * physical path value in string or binary format from device tree and
 * the defined api's ( getEntityPathFromDevTree() - to get binary data and
 * getPhysicalPathFromDevTree() - to get string data of physical path) will
 * support to get those values from device tree.
 *
 * To use device tree need to do init required phal library (libpdbg) and
 * same can be achieved by using initPHAL() api.
 */

#include "guard_common.hpp"

#include <optional>

namespace openpower
{
namespace guard
{
namespace phal
{
/**
 * @brief To init phal library for use power system specific device tree
 *
 * @return void
 */
void initPHAL();

/**
 * @brief Get entity path value from device tree
 *
 * Used to get entity path format value by using raw data which is defined
 * in device tree and the raw data is getting by using physical path value
 * as string format which is expected format by device tree.
 *
 * @param[in] physicalPath to pass physical path value
 * @return EntityPath if found in device tree else NULL
 */
std::optional<EntityPath>
    getEntityPathFromDevTree(const std::string& physicalPath);

/**
 * @brief Get physical path from device tree by using EntityPath value
 *
 * Used to get physical path string format value by using physical path
 * raw data which is defined in device tree and the raw data is getting by
 * using entity path type value.
 *
 * @param[in] entityPath to pass entity path value
 * @return Physical path if found in device tree else NULL
 */
std::optional<std::string>
    getPhysicalPathFromDevTree(const EntityPath& entityPath);
} // namespace phal
} // namespace guard
} // namespace openpower

#endif /* __PHAL_DEVTREE_H */
