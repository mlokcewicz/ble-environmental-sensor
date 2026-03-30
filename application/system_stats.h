//------------------------------------------------------------------------------

/// @file system_stats.h
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#ifndef SYSTEM_STATS_H_
#define SYSTEM_STATS_H_

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#include <stdbool.h>

//------------------------------------------------------------------------------

/// @brief Prints system statistics such as thread stack usage, heap usage, CPU load, and uptime.
/// @details This function iterates through all threads to print their stack usage, retrieves and prints
int system_stats_print(void);

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif /* SYSTEM_STATS_H_ */

//------------------------------------------------------------------------------
