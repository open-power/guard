// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <stdarg.h>

#define GUARD_EMERG 0
#define GUARD_ALERT 1
#define GUARD_CRIT 2
#define GUARD_ERROR 3
#define GUARD_WARNING 4
#define GUARD_NOTICE 5
#define GUARD_INFO 6
#define GUARD_DEBUG 7

namespace openpower
{
namespace guard
{
namespace log
{
/**
 * @brief Log the traces
 * @param[in] int type of log i.e. DBG, ERR, INFO
 * @param[in] fmt trace string
 * @param[in] ... variable set of arguments can be passed like %d,%s etc
 * @return NULL
 **/
void guard_log(int loglevel, const char* fmt, ...);

} // namespace log
} // namespace guard
} // namespace openpower
