// SPDX-License-Identifier: Apache-2.0
#include "config.h"

#include "guard_log.hpp"

#include <iostream>

namespace openpower
{
namespace guard
{
namespace log
{

void guard_log(int loglevel, const char* fmt, ...)
{
    if (VERBOSE_LEVEL < loglevel)
    {
        return;
    }

    va_list ap;
    va_start(ap, fmt);

    vprintf(fmt, ap);
    std::cout << "\n";

    va_end(ap);
}

} // namespace log
} // namespace guard
} // namespace openpower
