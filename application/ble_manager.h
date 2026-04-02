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

#include <stdbool.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

//------------------------------------------------------------------------------

/// @brief Initializes the Bluetooth subsystem and starts advertising.
/// @return 0 on success, or a negative error code on failure.
int ble_manager_init(void);

/// @brief Updates the advertising data to reflect a button press event.
/// @param button_pressed true if the button is pressed, false otherwise
/// @return 0 on success, or a negative error code on failure.
int ble_manager_advertise_button_pressed(bool button_pressed);

/// @brief Main processing loop for the BLE Manager. This function should be called from the main thread.
/// @note This function should be called periodically
/// @return 0 on success, or a negative error code on failure.
int ble_manager_process(void);

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif /* BLE_MANAGER_H_ */

//------------------------------------------------------------------------------
