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

#include "attributes_info.H"

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
 * @brief To get physical path binary value from device tree
 *
 * pdbg callback function used to get physical path binary value from
 * device tree by using physical path becuase guard doesn't know target
 * class name for given physical path.
 *
 * @param[in] target current target
 * @param[in] priv private data assoicated with callback function. not used
 * @return 0 to continue traverse, non-zero to stop traverse
 */
int pdbgCallbackToGetPhysicalBinaryPath(struct pdbg_target* target, void* priv);

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
} // namespace phal
} // namespace guard
} // namespace openpower

#endif /* __PHAL_DEVTREE_H */
