// SPDX-License-Identifier: Apache-2.0
#pragma once

#include "guard_exception.hpp"
#include "guard_log.hpp"

#include <array>
#include <cstdint>
#include <optional>

namespace openpower
{
namespace guard
{

using namespace openpower::guard::exception;

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

    EntityPath() = default;

    EntityPath(std::initializer_list<uint8_t> rawData)
    {
        if (rawData.size() > sizeof(EntityPath))
        {
            openpower::guard::log::guard_log(
                GUARD_ERROR,
                "Size mismatch. Given buf size[%d] EntityPath sizeof[%d]",
                rawData.size(), sizeof(EntityPath));
            throw InvalidEntityPath(
                "EntityPath initializer_list constructor failed");
        }

        if (rawData.size() == 0)
        {
            openpower::guard::log::guard_log(GUARD_ERROR,
                                             "Given raw data is empty");
            throw InvalidEntityPath(
                "EntityPath initializer_list constructor failed");
        }

        type_size = *(rawData.begin());

        if ((type_size & 0x0F) != ((rawData.size() - 1) / sizeof(PathElement)))
        {
            openpower::guard::log::guard_log(
                GUARD_ERROR, "PathElement size mismatch in given raw data");
            throw InvalidEntityPath(
                "EntityPath initializer_list constructor failed");
        }

        int i = 0;
        for (auto it = rawData.begin() + 1; it != rawData.end();
             std::advance(it, sizeof(PathElement)), i++)
        {
            if (std::distance(it, rawData.end()) <
                static_cast<int>(sizeof(PathElement)))
            {
                openpower::guard::log::guard_log(
                    GUARD_ERROR,
                    "Insufficient data for PathElement in given raw data");
                throw InvalidEntityPath(
                    "EntityPath initializer_list constructor failed");
            }

            pathElements[i].targetType = *it;
            pathElements[i].instance = *(it + 1);
        }
    }

    EntityPath(const uint8_t* rawData, size_t maxBufSize)
    {
        if (rawData == nullptr || maxBufSize == 0)
        {
            openpower::guard::log::guard_log(GUARD_ERROR,
                                             "Given raw data is empty");
            throw InvalidEntityPath("EntityPath conversion constructor failed");
        }

        if (maxBufSize > sizeof(EntityPath))
        {
            openpower::guard::log::guard_log(
                GUARD_ERROR,
                "Size mismatch. Given buf size[%d] EntityPath sizeof[%d]",
                maxBufSize, sizeof(EntityPath));
            throw InvalidEntityPath(
                "EntityPath initializer_list constructor failed");
        }

        type_size = rawData[0];

        uint8_t pathElementsSize = (type_size & 0x0F);
        size_t maxElements = ((maxBufSize - 1) / sizeof(PathElement));

        if (pathElementsSize > maxElements)
        {
            openpower::guard::log::guard_log(
                GUARD_ERROR,
                "PathElement size %d max elements size %d mismatch "
                "in given raw data",
                pathElementsSize, maxElements);
            throw InvalidEntityPath(
                "EntityPath initializer_list constructor failed");
        }

        for (int i = 0, j = 1; i < pathElementsSize;
             i++, j += sizeof(PathElement))
        {
            pathElements[i].targetType = rawData[j];
            pathElements[i].instance = rawData[j + 1];
        }
    }
} __attribute__((__packed__));

} // namespace guard
} // namespace openpower
