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

#include <services/lb_service.h>
#include <services/thp_service.h>
#include <services/battery_service.h>

//------------------------------------------------------------------------------

typedef void (*ble_manager_connection_state_cb)(bool connected);
typedef void (*ble_manager_led_set_cb)(const bool led_state);
typedef bool (*ble_manager_button_get_cb)(void);

typedef uint32_t (*ble_manager_sampling_interval_get_cb_t)(void);
typedef void (*ble_manager_sampling_interval_set_cb_t)(uint32_t sampling_interval_ms);

struct ble_manager_cfg
{
    ble_manager_connection_state_cb connection_state_cb;

    ble_manager_led_set_cb led_set_cb;
    ble_manager_button_get_cb button_get_cb;

    ble_manager_sampling_interval_get_cb_t sampling_interval_get_cb;
    ble_manager_sampling_interval_set_cb_t sampling_interval_set_cb;
};

//------------------------------------------------------------------------------

/// @brief Initializes the Bluetooth subsystem and starts advertising.
/// @param cfg pointer to the configuration structure
/// @return 0 on success, or a negative error code on failure.
int ble_manager_init(struct ble_manager_cfg *cfg);

/// @brief Updates the advertising data to reflect a button press event.
/// @param button_pressed true if the button is pressed, false otherwise
/// @return 0 on success, or a negative error code on failure.
int ble_manager_advertise_button_pressed(bool button_pressed);

/// @brief Unpairs the currently connected Bluetooth peer, if any.
/// @return 0 on success, or a negative error code on failure.
int ble_manager_unpair(void);

/// @brief Enters pairing mode, allowing new devices to pair with this device.
/// @return 0 on success, or a negative error code on failure.
int ble_manager_enter_pairing_mode(void);

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif /* BLE_MANAGER_H_ */

//------------------------------------------------------------------------------
