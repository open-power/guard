// SPDX-License-Identifier: Apache-2.0
#include "phal_devtree.hpp"

#include "guard_log.hpp"

#include <algorithm>
#include <cstring>
#include <stdexcept>

#include "attributes_info.H"

namespace openpower
{
namespace guard
{
namespace phal
{
namespace log = openpower::guard::log;

/**
 * Used to return in callback function which are used to get
 * physical path value and it binary format value.
 *
 * The value for constexpr defined based on pdbg_target_traverse function usage.
 */
constexpr int continueTgtTraversal = 0;
constexpr int requireAttrFound = 1;
constexpr int requireAttrNotFound = 2;

/**
 * Used to store|retrieve physical path in binary format inside pdbg
 * callback functions to get value from device tree
 */
static ATTR_PHYS_BIN_PATH_Type g_physBinaryPath;

/**
 * Used to store|retrieve physical path in string format inside pdbg
 * callback functions to get value from device tree
 */
static ATTR_PHYS_DEV_PATH_Type g_physStringPath;

void initPHAL()
{
    // Set log level to info
    pdbg_set_loglevel(PDBG_ERROR);

    /**
     * Passing fdt argument as NULL so, pdbg will use PDBG_DTB environment
     * variable to get system device tree.
     */
    if (!pdbg_targets_init(NULL))
    {
        log::guard_log(GUARD_ERROR, "pdbg_targets_init failed");
        throw std::runtime_error("pdbg target initialization failed");
    }
}

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
int pdbgCallbackToGetPhysicaBinaryPath(struct pdbg_target* target,
                                       void* /*priv - unused*/)
{
    ATTR_PHYS_DEV_PATH_Type physStringPath;
    if (DT_GET_PROP(ATTR_PHYS_DEV_PATH, target, physStringPath))
    {
        /**
         * Continue target traversal if ATTR_PHYS_DEV_PATH
         * attribute not found.
         */
        return continueTgtTraversal;
    }

    if (std::strcmp(g_physStringPath, physStringPath) != 0)
    {
        /**
         * Continue target traversal if ATTR_PHYS_DEV_PATH
         * attribute value is not matched with given physical path value.
         */
        return continueTgtTraversal;
    }
    else
    {
        /**
         * The ATTR_PHYS_DEV_PATH attribute value is matched with given
         * physical path value. so, getting physical path binary value
         * by using ATTR_PHYS_BIN_PATH attribute from same target property
         * list.
         */

        // Clear old value in g_physBinaryPath
        std::memset(g_physBinaryPath, 0, sizeof(g_physBinaryPath));

        if (DT_GET_PROP(ATTR_PHYS_BIN_PATH, target, g_physBinaryPath))
        {
            /**
             * Stopping the target target traversal if ATTR_PHYS_BIN_PATH
             * attribute not found within ATTR_PHYS_DEV_PATH attribute
             * target property list
             */
            return requireAttrNotFound;
        }
        else
        {
            /**
             * Found the physical binary value from device tree by using
             * given physical path value.
             */
            return requireAttrFound;
        }
    }

    return requireAttrNotFound;
}

std::optional<EntityPath>
    getEntityPathFromDevTree(const std::string& physicalPath)
{
    /**
     * Make sure physicalPath is in lower case because,
     * in device tree physical path is in lower case
     */
    std::string l_physicalPath(physicalPath);
    std::transform(l_physicalPath.begin(), l_physicalPath.end(),
                   l_physicalPath.begin(), ::tolower);

    /**
     * Removing "/" as prefix in given path if found to match with
     * device tree value
     */
    if (!l_physicalPath.compare(0, 1, "/"))
    {
        l_physicalPath.erase(0, 1);
    }

    /**
     * Adding "physical:" as prefix to given path to match with
     * device tree value if not given
     */
    if (l_physicalPath.find("physical:", 0, 9) == std::string::npos)
    {
        l_physicalPath.insert(0, "physical:");
    }

    if ((l_physicalPath.length() - 1 /* To include NULL terminator */) >
        sizeof(g_physStringPath))
    {
        log::guard_log(
            GUARD_ERROR,
            "Physical path size mismatch with given[%zu] and max size[%zu]",
            sizeof(g_physStringPath), (l_physicalPath.length() - 1));
        return std::nullopt;
    }

    /**
     * The callback function (pdbgCallbackToGetPhysicaBinaryPath) will use
     * the given physical path from g_physStringPath variable.
     * so, clear old value in g_physStringPath
     */
    std::memset(g_physStringPath, 0, sizeof(g_physStringPath));

    /**
     * The caller given value of physical path will be below format if not
     * in device tree format to get raw data of physical path.
     * E.g: physical:sys-0/node-0/proc-0
     */
    std::strncpy(g_physStringPath, l_physicalPath.c_str(),
                 sizeof(g_physStringPath) - 1);

    int ret = pdbg_target_traverse(
        nullptr /* Passing NULL to start target traversal from root */,
        pdbgCallbackToGetPhysicaBinaryPath,
        nullptr /* No application private data, so passing NULL */);
    if (ret == 0)
    {
        log::guard_log(
            GUARD_ERROR,
            "Given physical path not found in power system device tree");
        return std::nullopt;
    }
    else if (ret == requireAttrNotFound)
    {
        log::guard_log(
            GUARD_ERROR,
            "Binary value for given physical path is not found in device tree");
        return std::nullopt;
    }

    if (sizeof(EntityPath) < sizeof(g_physBinaryPath))
    {
        log::guard_log(
            GUARD_ERROR,
            "Physical path binary size mismatch with devtree[%zu] guard[%zu]",
            sizeof(g_physBinaryPath), sizeof(EntityPath));
        return std::nullopt;
    }

    return EntityPath(reinterpret_cast<uint8_t*>(g_physBinaryPath),
                      sizeof(g_physBinaryPath));
}

/**
 * @brief To get physical path from device tree
 *
 * pdbg callback function to get physical path from device tree
 * by using entity path raw data becuase guard doesn't know target
 * class name for given entity path
 *
 * @param[in] target current target
 * @param[in] priv private data assoicated with callback function. not used
 * @return 0 to continue traverse, non-zero to stop traverse
 */
int pdbgCallbackToGetPhysicalPath(struct pdbg_target* target,
                                  void* /*priv - unused*/)
{
    ATTR_PHYS_BIN_PATH_Type physBinaryPath;
    /**
     * TODO: Issue: phal/pdata#16
     * Should not use direct pdbg api to read attribute. Need to use
     * DT_GET_PROP but, libdt-api printing "pdbg_target_get_attribute failed"
     * for failure case and in guard case it expected trace because, doing
     * target iteration to get actual ATTR_PHYS_BIN_PATH value. So, Due to
     * this error trace user will get confusion while listing guard records.
     * Hence using pdbg api to avoid trace until libdt-api providing log
     * level setup.
     */
    if (!pdbg_target_get_attribute(
            target, "ATTR_PHYS_BIN_PATH",
            std::stoi(dtAttr::fapi2::ATTR_PHYS_BIN_PATH_Spec),
            dtAttr::fapi2::ATTR_PHYS_BIN_PATH_ElementCount, physBinaryPath))
    {
        /**
         * Continue target traversal if ATTR_PHYS_BIN_PATH
         * attribute not found.
         */
        return continueTgtTraversal;
    }

    for (size_t i = 0; i < sizeof(g_physBinaryPath); i++)
    {
        if (g_physBinaryPath[i] != physBinaryPath[i])
        {
            /**
             * Continue target traversal if ATTR_PHYS_BIN_PATH
             * attribute value is not matched with given physical path
             * binary format value.
             */
            return continueTgtTraversal;
        }
    }

    /**
     * The ATTR_PHYS_BIN_PATH attribute value is matched with given
     * physical path binary format value. so, getting physical path
     * value by using ATTR_PHYS_DEV_PATH attribute from same target
     * property list.
     */
    // clear old value in g_physStringPath
    std::memset(g_physStringPath, 0, sizeof(g_physStringPath));

    if (DT_GET_PROP(ATTR_PHYS_DEV_PATH, target, g_physStringPath))
    {
        /**
         * Stopping the target traversal if ATTR_PHYS_DEV_PATH
         * attribute not found within ATTR_PHYS_BIN_PATH attribute
         * target property list
         */
        return requireAttrNotFound;
    }
    else
    {
        /**
         * Found the physical path value from device tree by using
         * given physical path binary value.
         */
        return requireAttrFound;
    }
}

std::optional<std::string>
    getPhysicalPathFromDevTree(const EntityPath& entityPath)
{
    /**
     * The callback function (pdbgCallbackToGetPhysicalPath) will use
     * the given physical binary path from g_physBinaryPath variable.
     * so, clearing old value in g_physBinaryPath.
     */
    std::memset(g_physBinaryPath, 0, sizeof(g_physBinaryPath));

    if (sizeof(EntityPath) > sizeof(g_physBinaryPath))
    {
        log::guard_log(
            GUARD_ERROR,
            "Physical path binary size mismatch with devtree[%zu] guard[%zu]",
            sizeof(g_physBinaryPath), sizeof(EntityPath));
        return std::nullopt;
    }

    int rdIndex = 0;
    g_physBinaryPath[rdIndex++] = entityPath.type_size;

    /**
     * Path elements size stored at last 4bits in type_size member.
     * For every iteration, increasing 2 byte to store PathElement value
     * i.e target type enum value and instance id as raw data.
     *
     * @note PathElement targetType and instance member data type are
     * uint8_t. so, directly assiging each value as one byte to make
     * raw data (binary) format. If data type are changed for those fields
     * in PathElements then this logic also need to revisit.
     */
    for (int i = 0; i < (0x0F & entityPath.type_size);
         i++, rdIndex += sizeof(entityPath.pathElements[0]))
    {
        g_physBinaryPath[rdIndex] = entityPath.pathElements[i].targetType;
        g_physBinaryPath[rdIndex + 1] = entityPath.pathElements[i].instance;
    }

    int ret = pdbg_target_traverse(
        nullptr /* Passing NULL to start target traversal from root */,
        pdbgCallbackToGetPhysicalPath,
        nullptr /* No application private data, so passing NULL */);

    if (ret == 0)
    {
        log::guard_log(
            GUARD_ERROR,
            "Given binary physical path not found in power system device tree");
        return std::nullopt;
    }
    else if (ret == requireAttrNotFound)
    {
        log::guard_log(GUARD_ERROR, "String value for binary physical path is "
                                    "not found in device tree");
        return std::nullopt;
    }

    return std::string(g_physStringPath, sizeof(g_physStringPath));
}
} // namespace phal
} // namespace guard
} // namespace openpower
