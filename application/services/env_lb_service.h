//------------------------------------------------------------------------------

/// @file env_lb_service.h
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#ifndef ENV_LB_SERVICE_H_
#define ENV_LB_SERVICE_H_

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#include <stdbool.h>

#include <zephyr/types.h>

//------------------------------------------------------------------------------

#define BT_UUID_ENV_LBS_VAL BT_UUID_128_ENCODE(0x00001523, 0x1212, 0xefde, 0x1523, 0x785feabcd123)
#define BT_UUID_ENV_LBS_BUTTON_VAL BT_UUID_128_ENCODE(0x00001524, 0x1212, 0xefde, 0x1523, 0x785feabcd123)
#define BT_UUID_ENV_LBS_LED_VAL BT_UUID_128_ENCODE(0x00001525, 0x1212, 0xefde, 0x1523, 0x785feabcd123)
#define BT_UUID_ENV_LBS_MYSENSOR_VAL BT_UUID_128_ENCODE(0x00001526, 0x1212, 0xefde, 0x1523, 0x785feabcd123)
#define BT_UUID_ENV_LBS_SAMPLING_INTERVAL_VAL BT_UUID_128_ENCODE(0x00001527, 0x1212, 0xefde, 0x1523, 0x785feabcd123)

#define BT_UUID_ENV_LBS BT_UUID_DECLARE_128(BT_UUID_ENV_LBS_VAL)
#define BT_UUID_ENV_LBS_BUTTON BT_UUID_DECLARE_128(BT_UUID_ENV_LBS_BUTTON_VAL)
#define BT_UUID_ENV_LBS_LED BT_UUID_DECLARE_128(BT_UUID_ENV_LBS_LED_VAL)
#define BT_UUID_ENV_LBS_MYSENSOR BT_UUID_DECLARE_128(BT_UUID_ENV_LBS_MYSENSOR_VAL)
#define BT_UUID_ENV_LBS_SAMPLING_INTERVAL BT_UUID_DECLARE_128(BT_UUID_ENV_LBS_SAMPLING_INTERVAL_VAL)

//------------------------------------------------------------------------------

/// @brief Callback type for when an LED state change is received.
/// @param led_state the new state of the LED (true for on, false for off)
typedef void (*led_cb_t)(const bool led_state);

/// @brief Callback type for when the button state is pulled.
/// @return The state of the button (true for pressed, false for not pressed)
typedef bool (*button_cb_t)(void);

/// @brief Callback type for when the sampling interval is pulled.
/// @return The current sampling interval in milliseconds.
typedef uint32_t (*sampling_interval_get_cb_t)(void);

/// @brief Callback type for when the sampling interval is pulled or updated.
/// @param sampling_interval_ms The sampling interval in milliseconds (for set callback).
typedef void (*sampling_interval_set_cb_t)(uint32_t sampling_interval_ms);

/// @brief Callback struct used by the LBS Service.
struct env_lb_service_cb 
{
	led_cb_t led_set_cb;
	button_cb_t button_get_cb;
	sampling_interval_get_cb_t sampling_interval_get_cb;
	sampling_interval_set_cb_t sampling_interval_set_cb;
};

//------------------------------------------------------------------------------

/// @brief Initialize the ENV LB Service.
/// @note This function registers application callback functions with the ENV LB Service
/// @param[in] callbacks Struct containing pointers to callback functions used by the service. 
/// @return 0 If the operation was successful. Otherwise, a (negative) error code is returned.
int env_lb_service_init(struct env_lb_service_cb *callbacks);

/// @brief Send the button state as indication.
/// @note This function sends a binary state, typically the state of a button, to all connected peers.
/// @param[in] button_state The state of the button.
/// @return 0 If the operation was successful. Otherwise, a (negative) error code is returned.
int env_lb_service_send_button_state_indicate(bool button_state);

/// @brief Send the button state as notification.
/// This function sends a binary state, typically the state of a button, to all connected peers.
/// @param[in] button_state The state of the button.
/// @return 0 If the operation was successful. Otherwise, a (negative) error code is returned.
int env_lb_service_send_button_state_notify(bool button_state);

/// @brief Send the sensor value as notification.
/// @note This function sends an uint32_t a sensor to all connected peers.
/// @param[in] sensor_value The value of the simulated sensor.
/// @return 0 If the operation was successful. Otherwise, a (negative) error code is returned.
int env_lb_service_send_sensor_notify(uint32_t sensor_value);

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif /* ENV_LB_SERVICE_H_ */

//------------------------------------------------------------------------------
