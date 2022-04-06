// SPDX-License-Identifier: Apache-2.0
#include "config.h"

#include "guard_entity.hpp"

#include "guard_entity_map.hpp"

#ifdef DEV_TREE
#include "phal_devtree.hpp"
#endif /* DEV_TREE */

namespace openpower
{
namespace guard
{
std::optional<EntityPath> getEntityPath(const std::string& physicalPath)
{
#ifdef DEV_TREE

    return openpower::guard::phal::getEntityPathFromDevTree(physicalPath);

#else  // from custom list
    auto it = physicalEntityPathMap.find(physicalPath);
    if (it != physicalEntityPathMap.end())
    {
        return it->second;
    }
    return std::nullopt;
#endif /* DEV_TREE */
}

std::optional<std::string> getPhysicalPath(const EntityPath& entityPath)
{
#ifdef DEV_TREE

    return openpower::guard::phal::getPhysicalPathFromDevTree(entityPath);

#else  // from custom list
    for (auto i : physicalEntityPathMap)
    {
        if (i.second == entityPath)
        {
            return i.first;
        }
    }
    return std::nullopt;
#endif /* DEV_TREE */
}

std::optional<std::string> pathTypeToString(const int pType)
{
    auto i = PathType.find(pType);
    if (i != PathType.end())
    {
        return i->second;
    }

    return std::nullopt;
}

std::string guardReasonToStr(const int gReason)
{
    auto i = guardreason.find(gReason);
    if (i != guardreason.end())
    {
        return i->second;
    }
    std::string unknownStr{"unknown [" + std::to_string(gReason) + "]"};

    return unknownStr;
}
} // namespace guard
} // namespace openpower
