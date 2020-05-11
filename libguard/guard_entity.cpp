// SPDX-License-Identifier: Apache-2.0
#include "guard_entity.hpp"

#include "guard_entity_map.hpp"

namespace openpower
{
namespace guard
{
std::optional<EntityPath> getEntityPath(const std::string& physicalPath)
{
#ifdef PHAL
    // TODO: use device tree
#else
    auto it = physicalEntityPathMap.find(physicalPath);
    if (it != physicalEntityPathMap.end())
    {
        return it->second;
    }
#endif
    return std::nullopt;
}

std::optional<std::string> getPhysicalPath(const EntityPath& entityPath)
{
#ifdef PHAL
    // TODO: use device tree
#else
    for (auto i : physicalEntityPathMap)
    {
        if (i.second == entityPath)
        {
            return i.first;
        }
    }
#endif
    return std::nullopt;
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

std::optional<std::string> guardReasonToStr(const int gReason)
{
    auto i = guardreason.find(gReason);
    if (i != guardreason.end())
    {
        return i->second;
    }
    return std::nullopt;
}
} // namespace guard
} // namespace openpower
