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
#include <stdint.h>

//------------------------------------------------------------------------------

/// @brief Initializes the Environment Manager module.
/// @return 0 on success, or a negative error code on failure.
int env_manager_init(void);

/// @brief  Sets the sampling interval for the environment sensor data.
/// @param sampling_interval_ms The desired sampling interval in milliseconds.
/// @return 0 on success, or a negative error code on failure.
int env_manager_sampling_interval_set(uint32_t sampling_interval_ms);

/// @brief  Gets the current sampling interval for the environment sensor data.
/// @param sampling_interval_ms A pointer to a variable where the current sampling interval in milliseconds will be stored.
/// @return 0 on success, or a negative error code on failure.
int env_manager_sampling_interval_get(uint32_t *sampling_interval_ms);

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
