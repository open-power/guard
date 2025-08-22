// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "guard_common.hpp"
#include <attributes_info.H>
#include <map>

namespace openpower
{
namespace guard
{
/**
 *  @brief Entity Path Types
 *
 *  An entity path type indicates which specific set of relationships is
 *  modeled by a particular entity path.  For example, PATH_AFFINITY
 *  models how targets are connected to each other from a hardware
 *  affinity perspective.
 */
const static std::map<int, std::string> PathType = {{0x00, "notApplicable"},
                                                    {0x01, "affinity"},
                                                    {0x02, "physical"},
                                                    {0x03, "device"},
                                                    {0x04, "power"}};

using reason = std::map<int, std::string>;
const static reason guardreason = {
    {0x00, "noReason"},       {0xC0, "spare"},      {0xD2, "manual"},
    {0xE2, "unrecoverable"},  {0xE3, "fatal"},      {0xE6, "predictive"},
    {0xE9, "power"},          {0xEA, "hypervisor"}, {0xEB, "reconfig"},
    {0xEC, "sticky_deconfig"}};

/**
 * @brief Return entity path for the corresponding physical path
 *
 * @param[in] physicalPath physical path
 * @return NULL if entity path not found else entity path
 *              computed from physical path
 *
 */
std::optional<EntityPath> getEntityPath(const std::string& physicalPath);

/**
 * @brief Return physical path computed from  entity path
 *
 * @param[in] entityPath entity path
 * @return NULL if physical path is not found else physical path
 *              computed from entity path
 */
std::optional<std::string> getPhysicalPath(const EntityPath& entityPath);

/**
 * @brief Return string value for corresponding path type
 *
 * @param[in] pType path type
 * @return NULL if path type is not found else path type
 */
std::optional<std::string> pathTypeToString(const int pType);

/**
 * @brief Return string value for corresponding guard reason
 *
 * @param[in] gReason guard reason
 *
 * @return "unknown" with the given guard reason if not found
 *         else corresponding guard reason string.
 */
std::string guardReasonToStr(const int gReason);

/**
 * @brief get the target type based as per type defined in attributes_info.H
 *
 * @param[in] entityPath The entity path struct
 *
 * @return "ATTR_TYPE_Enum" respective enum value of the target
 */
ATTR_TYPE_Enum getTargetType(const EntityPath& entityPath);

/**
 * @brief Returns true if it is a core or fc
 *
 * @param[in] const targetType the target type as per attrbutes_info.H
 *
 * @return bool value - true if it is core
 *                      false if any other target type
 */
inline bool isCore(const ATTR_TYPE_Enum targetType)
{
    return( (targetType == ENUM_ATTR_TYPE_CORE) || 
    (targetType == ENUM_ATTR_TYPE_FC) );
}

} // namespace guard
} // namespace openpower
