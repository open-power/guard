// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <array>

namespace openpower
{
namespace guard
{
/* From hostboot: src/include/usr/targeting/common/entitypath.H */
struct EntityPath
{
    /**
     * @brief Entity Path Element Definition
     *
     * Any entity path models one level of an entity path hierarchy
     */
    struct PathElement
    {
        uint8_t targetType; ///< Type of element at this level in the hierarchy
        uint8_t instance;   ///< Instance ID for the element, relative to
                            ///< the parent
    } __attribute__((__packed__));

    uint8_t type_size;

    static const int maxPathElements = 10;
    using PathElements = std::array<PathElement, maxPathElements>;
    PathElements pathElements;

    bool operator==(const EntityPath& a) const
    {
        if ((a.type_size & 0x0F) != (type_size & 0x0F))
        {
            return false;
        }

        if ((a.type_size & 0xF0) != (type_size & 0xF0))
        {
            return false;
        }

        if ((a.type_size & 0x0F) > maxPathElements)
        {
            return false;
        }

        for (int i = 0; i < (type_size & 0x0F); i++)
        {
            if (a.pathElements[i].targetType != pathElements[i].targetType)
            {
                return false;
            }
            if (a.pathElements[i].instance != pathElements[i].instance)
            {
                return false;
            }
        }

        return true;
    }
} __attribute__((__packed__));

} // namespace guard
} // namespace openpower
