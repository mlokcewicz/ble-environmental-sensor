//------------------------------------------------------------------------------

/// @file thp_service.h
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#ifndef THP_SERVICE_H_
#define THP_SERVICE_H_

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#include <stdbool.h>

#include <zephyr/types.h>

//------------------------------------------------------------------------------

#define BT_UUID_THP_VAL BT_UUID_128_ENCODE(0xd5d63513, 0x0971, 0x4e4f, 0xabfa, 0x4b37c97f40c3)
#define BT_UUID_THP_TEMP_VAL BT_UUID_128_ENCODE(0xd5d63514, 0x0971, 0x4e4f, 0xabfa, 0x4b37c97f40c3)
#define BT_UUID_THP_HUMIDITY_VAL BT_UUID_128_ENCODE(0xd5d63515, 0x0971, 0x4e4f, 0xabfa, 0x4b37c97f40c3)
#define BT_UUID_THP_PRESSURE_VAL BT_UUID_128_ENCODE(0xd5d63516, 0x0971, 0x4e4f, 0xabfa, 0x4b37c97f40c3)
#define BT_UUID_THP_SAMPLING_INTERVAL_VAL BT_UUID_128_ENCODE(0xd5d63517, 0x0971, 0x4e4f, 0xabfa, 0x4b37c97f40c3)

#define BT_UUID_THP BT_UUID_DECLARE_128(BT_UUID_THP_VAL)
#define BT_UUID_THP_TEMP BT_UUID_DECLARE_128(BT_UUID_THP_TEMP_VAL)
#define BT_UUID_THP_HUMIDITY BT_UUID_DECLARE_128(BT_UUID_THP_HUMIDITY_VAL)
#define BT_UUID_THP_PRESSURE BT_UUID_DECLARE_128(BT_UUID_THP_PRESSURE_VAL)
#define BT_UUID_THP_SAMPLING_INTERVAL BT_UUID_DECLARE_128(BT_UUID_THP_SAMPLING_INTERVAL_VAL)

//------------------------------------------------------------------------------

enum thp_service_sensor_type 
{
	THP_SENSOR_TYPE_TEMP,
	THP_SENSOR_TYPE_HUMIDITY,
	THP_SENSOR_TYPE_PRESSURE,

	THP_SENSOR_TYPE_COUNT
};

//------------------------------------------------------------------------------

/// @brief Callback type for when the sampling interval is pulled.
/// @return The current sampling interval in milliseconds.
typedef uint32_t (*sampling_interval_get_cb_t)(void);

/// @brief Callback type for when the sampling interval is updated.
/// @param sampling_interval_ms The sampling interval in milliseconds (for set callback).
typedef void (*sampling_interval_set_cb_t)(uint32_t sampling_interval_ms);

/// @brief Callback struct used by the LBS Service.
struct thp_service_cb 
{
	sampling_interval_get_cb_t sampling_interval_get_cb;
	sampling_interval_set_cb_t sampling_interval_set_cb;
};

//------------------------------------------------------------------------------

/// @brief Initialize the THP Service.
/// @note This function registers application callback functions with the THP Service
/// @param[in] callbacks Struct containing pointers to callback functions used by the service. 
/// @return 0 If the operation was successful. Otherwise, a (negative) error code is returned.
int thp_service_init(struct thp_service_cb *callbacks);

/// @brief Send the sensor value as notification.
/// @note This function sends an uint32_t a sensor to all connected peers.
/// @param[in] sensor_type The type of the sensor value being sent (temperature, humidity, or pressure).
/// @param[in] sensor_value The value of the simulated sensor.
/// @return 0 If the operation was successful. Otherwise, a (negative) error code is returned.
int thp_service_send_sensor_notify(enum thp_service_sensor_type sensor_type, uint32_t sensor_value);

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif /* THP_SERVICE_H_ */

//------------------------------------------------------------------------------
