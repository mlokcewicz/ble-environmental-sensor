//------------------------------------------------------------------------------

/// @file ble_manager.h
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#ifndef BLE_MANAGER_H_
#define BLE_MANAGER_H_

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <stdbool.h>

//------------------------------------------------------------------------------

typedef void (*ble_manager_connection_state_cb)(bool connected);

//------------------------------------------------------------------------------

/// @brief Initializes the Bluetooth subsystem and starts advertising.
/// @param connection_state_cb connection state callback
/// @return 0 on success, or a negative error code on failure.
int ble_manager_init(ble_manager_connection_state_cb connection_state_cb);


/// @brief Updates the advertising data to reflect a button press event.
/// @param button_pressed true if the button is pressed, false otherwise
/// @return 0 on success, or a negative error code on failure.
int ble_manager_notify_button_pressed(bool button_pressed);

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif /* BLE_MANAGER_H_ */

//------------------------------------------------------------------------------
