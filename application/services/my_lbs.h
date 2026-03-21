//------------------------------------------------------------------------------

/// @file my_lbs.h
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#ifndef MY_LBS_H_
#define MY_LBS_H_

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#include <stdbool.h>

#include <zephyr/types.h>

//------------------------------------------------------------------------------

#define BT_UUID_LBS_VAL BT_UUID_128_ENCODE(0x00001523, 0x1212, 0xefde, 0x1523, 0x785feabcd123)
#define BT_UUID_LBS_BUTTON_VAL BT_UUID_128_ENCODE(0x00001524, 0x1212, 0xefde, 0x1523, 0x785feabcd123)
#define BT_UUID_LBS_LED_VAL BT_UUID_128_ENCODE(0x00001525, 0x1212, 0xefde, 0x1523, 0x785feabcd123)
  
#define BT_UUID_LBS BT_UUID_DECLARE_128(BT_UUID_LBS_VAL)
#define BT_UUID_LBS_BUTTON BT_UUID_DECLARE_128(BT_UUID_LBS_BUTTON_VAL)
#define BT_UUID_LBS_LED BT_UUID_DECLARE_128(BT_UUID_LBS_LED_VAL)

//------------------------------------------------------------------------------

/// @brief Callback type for when an LED state change is received.
/// @param led_state the new state of the LED (true for on, false for off)
typedef void (*led_cb_t)(const bool led_state);

/// @brief Callback type for when the button state is pulled.
/// @return The state of the button (true for pressed, false for not pressed)
typedef bool (*button_cb_t)(void);

/// @brief Callback struct used by the LBS Service.
struct my_lbs_cb 
{
	led_cb_t led_set_cb;
	button_cb_t button_get_cb;
};

//------------------------------------------------------------------------------

/// @brief Initialize the LBS Service.
/// @note This function registers application callback functions with the My LBS Service
/// @param[in] callbacks Struct containing pointers to callback functions used by the service. 
/// @return 0 If the operation was successful. Otherwise, a (negative) error code is returned.
int my_lbs_init(struct my_lbs_cb *callbacks);

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif /* MY_LBS_H_ */

//------------------------------------------------------------------------------
