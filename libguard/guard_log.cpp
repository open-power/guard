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
    va_list ap;

    va_start(ap, fmt);
    if (loglevel == GUARD_ERR)
    {
        vprintf(fmt, ap);
    }
    else if (loglevel == GUARD_INF)
    {
        vprintf(fmt, ap);
    }
    else if (loglevel == GUARD_DBG)
    {
        vprintf(fmt, ap);
    }
    va_end(ap);
    std::cout << "\n";
}

} // namespace log
} // namespace guard
} // namespace openpower
