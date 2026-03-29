//------------------------------------------------------------------------------

/// @file battery_service.h
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#ifndef BATTERY_SERVICE_H_
#define BATTERY_SERVICE_H_

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#include <stdbool.h>

#include <zephyr/types.h>

//------------------------------------------------------------------------------

/// @brief Send the battery level as notification.
/// @param[in] battery_level The value of the battery level.
/// @return 0 If the operation was successful. Otherwise, a (negative) error code is returned.
int battery_service_send_battery_notify(uint8_t battery_level);

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif /* BATTERY_SERVICE_H_ */

//------------------------------------------------------------------------------
