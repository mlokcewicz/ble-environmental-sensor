//------------------------------------------------------------------------------

/// @file env_manager.h
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#ifndef ENV_MANAGER_H_
#define ENV_MANAGER_H_

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#include <stdbool.h>

//------------------------------------------------------------------------------

/// @brief Initializes the Environment Manager module.
/// @return 0 on success, or a negative error code on failure.
int env_manager_init(void);

/// @brief Processes Environment Manager tasks (sensor data sampling)
/// @note This function should be called periodically
/// @return 0 on success, or a negative error code on failure.
int env_manager_process(void);

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif /* ENV_MANAGER_H_ */

//------------------------------------------------------------------------------
