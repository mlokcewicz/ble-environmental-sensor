//------------------------------------------------------------------------------

/// @file app_event.h
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#ifndef APP_EVENT_H_
#define APP_EVENT_H_

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#include <stdbool.h>

#include <zephyr/kernel.h>
#include <zephyr/zbus/zbus.h>

//------------------------------------------------------------------------------

#define APP_EVENT_SENSOR_DATA_QUEUE_SIZE 10

//------------------------------------------------------------------------------

enum app_event_type
{
    // From BLE Manager
    APP_EVENT_BLE_CONNECTION_STATE_IND,
    APP_EVENT_LBS_LED_SET_REQ,
    APP_EVENT_SAMPLING_INTERVAL_SET_REQ,

    // From UI Manager
    APP_EVENT_BLE_UNPAIR_REQ, 
    APP_EVENT_BLE_PAIRING_MODE_REQ,
    APP_EVENT_LBS_BUTTON_STATE_IND,

    // From ENV Manager
    APP_EVENT_SENSOR_DATA_IND,

    // From Main
    APP_EVENT_BATTERY_LEVEL_IND,

    APP_EVENT_MAX,
};

struct app_event
{
    enum app_event_type type;
    union
    {
        bool ble_connected;            // for APP_EVENT_BLE_CONNECTION_STATE_IND
        bool led_state;                // for APP_EVENT_LBS_LED_SET_REQ
        uint32_t sampling_interval_ms; // for APP_EVENT_SAMPLING_INTERVAL_SET_REQ
        bool button_state;             // for APP_EVENT_LBS_BUTTON_STATE_IND
        uint8_t battery_level;         // for APP_EVENT_BATTERY_LEVEL_IND
        struct 
        {
            uint32_t temp_celsius_exp1;     // Temperature in Celsius with one decimal place (e.g., 253 means 25.3°C)
            uint32_t humidity_percent_exp1; // Humidity in % with one decimal place (e.g., 453 means 45.3%)
            uint32_t pressure_hpa_exp1;     // Pressure in hPa with one decimal place (e.g., 10132 means 1013.2 hPa)
        } sensor_data;                      // for APP_EVENT_SENSOR_DATA_IND
    };
};

//------------------------------------------------------------------------------

ZBUS_CHAN_DECLARE(ble_control_chan);
ZBUS_CHAN_DECLARE(env_control_chan);
ZBUS_CHAN_DECLARE(ui_control_chan);

extern struct k_msgq sensor_event_queue; 

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif /* APP_EVENT_H_ */

//------------------------------------------------------------------------------
