//------------------------------------------------------------------------------

/// @file thp_service.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include <stddef.h>
#include <string.h>
#include <errno.h>

#include <zephyr/types.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/byteorder.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>

#include <zephyr/logging/log.h>

#include "thp_service.h"

//------------------------------------------------------------------------------

struct thp_service_ctx 
{
    struct thp_service_cb thp_cb;
	bool notify_enabled[THP_SENSOR_TYPE_COUNT]; /* Array to track notification status for each sensor type */
	uint32_t sampling_interval_ms;
};

//------------------------------------------------------------------------------

LOG_MODULE_REGISTER(thp_service);

static struct thp_service_ctx ctx;

//------------------------------------------------------------------------------

static void mythpbc_ccc_temp_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	ctx.notify_enabled[THP_SENSOR_TYPE_TEMP] = (value == BT_GATT_CCC_NOTIFY);
}

static void mythpbc_ccc_humidity_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	ctx.notify_enabled[THP_SENSOR_TYPE_HUMIDITY] = (value == BT_GATT_CCC_NOTIFY);
}

static void mythpbc_ccc_pressure_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	ctx.notify_enabled[THP_SENSOR_TYPE_PRESSURE] = (value == BT_GATT_CCC_NOTIFY);
}

/* Read callback function of the sampling interval characteristic */
static ssize_t read_sampling_interval(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset)
{
    /* Get a pointer to sampling_interval which is passed in the BT_GATT_CHARACTERISTIC() and stored in attr->user_data */
	const uint32_t *value = attr->user_data;
    
	LOG_DBG("Attribute read, handle: %u, conn: %p", attr->handle, (void *)conn);
    
	if (ctx.thp_cb.sampling_interval_get_cb) 
    {
        /* Call the application callback function to update the get the current value of the sampling interval */
		ctx.sampling_interval_ms = ctx.thp_cb.sampling_interval_get_cb(); /* value points to ctx.sampling_interval_ms */
        
        /* Call the function bt_gatt_attr_read() to send the value (value -> buf) to the GATT client (the central device). */
		return bt_gatt_attr_read(conn, attr, buf, len, offset, value, sizeof(*value));
	}
    
	return 0;
}

/* Write callback function of the sampling interval characteristic */
static ssize_t write_sampling_interval(struct bt_conn *conn, const struct bt_gatt_attr *attr, const void *buf, uint16_t len, uint16_t offset, uint8_t flags)
{
	LOG_DBG("Attribute write, handle: %u, conn: %p", attr->handle, (void *)conn);

	if (len != sizeof(uint32_t)) 
    {
		LOG_DBG("Write sampling interval: Incorrect data length");
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
	}

	if (offset != 0) 
    {
		LOG_DBG("Write sampling interval: Incorrect data offset");
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	}

	if (ctx.thp_cb.sampling_interval_set_cb) 
    {
		/* Read the received value from the buf parameter. The value is expected to be a 32-bit integer representing the sampling interval in milliseconds. */
		uint32_t val = *((uint32_t *)buf);

		if (val == 0)
		{
			LOG_DBG("Write sampling interval: Incorrect value");
			return BT_GATT_ERR(BT_ATT_ERR_VALUE_NOT_ALLOWED);
		}
		
		/* Call the application callback function to update the sampling interval */
		ctx.thp_cb.sampling_interval_set_cb(val);
	}

	return len;
}

//------------------------------------------------------------------------------

static const struct bt_gatt_cpf temp_cpf =
{
    .format = 0x0E,   /* sint16 */
    .exponent = -1,   /* value * 10^-1, np. 253 -> 25.3 */
    .unit = 0x272F,   /* degree Celsius */
    .name_space = 1,  /* Bluetooth SIG */
    .description = 0,
};

static const struct bt_gatt_cpf humidity_cpf =
{
    .format = 0x0E,   // sint16
    .exponent = -1,   // 0.1 %
    .unit = 0x27AD,   // percentage
    .name_space = 1,
    .description = 0,
};


static const struct bt_gatt_cpf pressure_cpf =
{
    .format = 0x0E,   // sint16
    .exponent = 1,    // value in hPa
    .unit = 0x2724,   // Pascal
    .name_space = 1,
    .description = 0,
};

static const struct bt_gatt_cpf sampling_interval_cpf =
{
    .format = 0x08,     // uint32
    .exponent = -3, 	// value in ms
    .unit = 0x2703,     // second
    .name_space = 1,
    .description = 0,
};

/* LED Button Service Declaration - Create and add the MY THP service to the Bluetooth LE stack */
BT_GATT_SERVICE_DEFINE
(   
	thp_svc,
    BT_GATT_PRIMARY_SERVICE(BT_UUID_THP),
    BT_GATT_CHARACTERISTIC(BT_UUID_THP_TEMP, BT_GATT_CHRC_NOTIFY, BT_GATT_PERM_NONE, NULL, NULL, NULL),
	BT_GATT_CUD("Temperature in Celsius", BT_GATT_PERM_READ),
	BT_GATT_CPF(&temp_cpf),	
    BT_GATT_CCC(mythpbc_ccc_temp_cfg_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
	 
    BT_GATT_CHARACTERISTIC(BT_UUID_THP_HUMIDITY, BT_GATT_CHRC_NOTIFY, BT_GATT_PERM_NONE, NULL, NULL, NULL),
	BT_GATT_CUD("Humidity in Percent", BT_GATT_PERM_READ),
	BT_GATT_CPF(&humidity_cpf),
	BT_GATT_CCC(mythpbc_ccc_humidity_cfg_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE), 
	
    BT_GATT_CHARACTERISTIC(BT_UUID_THP_PRESSURE, BT_GATT_CHRC_NOTIFY, BT_GATT_PERM_NONE, NULL, NULL, NULL),
	BT_GATT_CUD("Pressure in Pa", BT_GATT_PERM_READ),
	BT_GATT_CPF(&pressure_cpf),
    BT_GATT_CCC(mythpbc_ccc_pressure_cfg_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE), 
	
	BT_GATT_CHARACTERISTIC(BT_UUID_THP_SAMPLING_INTERVAL, BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE_AUTHEN, read_sampling_interval, write_sampling_interval, &ctx.sampling_interval_ms),
	BT_GATT_CUD("Sampling Interval in ms", BT_GATT_PERM_READ),
	BT_GATT_CPF(&sampling_interval_cpf),
);

//------------------------------------------------------------------------------

/* A function to register application callbacks for the LED and Button characteristics  */
int thp_service_init(struct thp_service_cb *callbacks)
{
	if (callbacks) 
    {
		ctx.thp_cb.sampling_interval_get_cb = callbacks->sampling_interval_get_cb;
		ctx.thp_cb.sampling_interval_set_cb = callbacks->sampling_interval_set_cb;
	}

	return 0;
}

int thp_service_send_sensor_notify(enum thp_service_sensor_type sensor_type, uint32_t sensor_value)
{
	if (sensor_type >= THP_SENSOR_TYPE_COUNT) 
	{
		LOG_DBG("Invalid sensor type for notification");
		return -EINVAL;
	}

	if (!ctx.notify_enabled[sensor_type])
		return -EACCES;

	const struct bt_uuid *thp_sensor_uuids[] =
	{
		[THP_SENSOR_TYPE_TEMP] = BT_UUID_THP_TEMP,
		[THP_SENSOR_TYPE_HUMIDITY] = BT_UUID_THP_HUMIDITY,
		[THP_SENSOR_TYPE_PRESSURE] = BT_UUID_THP_PRESSURE,
	};

	return bt_gatt_notify_uuid(NULL, thp_sensor_uuids[sensor_type], &thp_svc.attrs[0], &sensor_value, sizeof(sensor_value));
}

//------------------------------------------------------------------------------
