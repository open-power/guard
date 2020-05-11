// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <stdarg.h>

#define GUARD_ERROR 1
#define GUARD_INFO 2
#define GUARD_DEBUG 3

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
