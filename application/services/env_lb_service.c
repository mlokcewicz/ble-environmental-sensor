//------------------------------------------------------------------------------

/// @file env_lb_service.c
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

#include "env_lb_service.h"

//------------------------------------------------------------------------------

struct env_lb_service_ctx 
{
    struct env_lb_service_cb lbs_cb;
    struct bt_gatt_indicate_params ind_params;
    bool button_state;
    bool notify_mysensor_enabled;
    bool indicate_button_enabled;
	uint32_t sampling_interval_ms;
};

//------------------------------------------------------------------------------

LOG_MODULE_DECLARE(env_lb_service);

static struct env_lb_service_ctx ctx;

//------------------------------------------------------------------------------

/* Read callback function of the Button characteristic */
static ssize_t read_button(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset)
{
    /* Get a pointer to button_state which is passed in the BT_GATT_CHARACTERISTIC() and stored in attr->user_data */
	const char *value = attr->user_data;
    
	LOG_DBG("Attribute read, handle: %u, conn: %p", attr->handle, (void *)conn);
    
	if (ctx.lbs_cb.button_get_cb) 
    {
        /* Call the application callback function to update the get the current value of the button */
		ctx.button_state = ctx.lbs_cb.button_get_cb(); /* value points to ctx.button_state */
        
        /* Call the function bt_gatt_attr_read() to send the value (value -> buf) to the GATT client (the central device). */
		return bt_gatt_attr_read(conn, attr, buf, len, offset, value, sizeof(*value));
	}
    
	return 0;
}

/* Write callback function of the LED characteristic */
static ssize_t write_led(struct bt_conn *conn, const struct bt_gatt_attr *attr, const void *buf, uint16_t len, uint16_t offset, uint8_t flags)
{
	LOG_DBG("Attribute write, handle: %u, conn: %p", attr->handle, (void *)conn);

	if (len != 1U) 
    {
		LOG_DBG("Write led: Incorrect data length");
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
	}

	if (offset != 0) 
    {
		LOG_DBG("Write led: Incorrect data offset");
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	}

	if (ctx.lbs_cb.led_set_cb) 
    {
		/* Read the received value from the buf parameter. The value is expected to be either 0x00 (LED off) or 0x01 (LED on). */ 
		uint8_t val = *((uint8_t *)buf);

		if (val == 0x00 || val == 0x01) 
        {
			/* Call the application callback function to update the LED state */
			ctx.lbs_cb.led_set_cb(val ? true : false);
		} 
        else 
        {
			LOG_DBG("Write led: Incorrect value");
			return BT_GATT_ERR(BT_ATT_ERR_VALUE_NOT_ALLOWED);
		}
	}

	return len;
}

static void indicate_ack_cb(struct bt_conn *conn, struct bt_gatt_indicate_params *params, uint8_t err)
{
	LOG_DBG("Indication %s\n", err != 0U ? "fail" : "success");
}

static void mylbsbc_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	ctx.indicate_button_enabled = (value == BT_GATT_CCC_INDICATE);
}

static void mylbsbc_ccc_mysensor_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	ctx.notify_mysensor_enabled = (value == BT_GATT_CCC_NOTIFY);
}

/* Read callback function of the sampling interval characteristic */
static ssize_t read_sampling_interval(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset)
{
    /* Get a pointer to sampling_interval which is passed in the BT_GATT_CHARACTERISTIC() and stored in attr->user_data */
	const uint32_t *value = attr->user_data;
    
	LOG_DBG("Attribute read, handle: %u, conn: %p", attr->handle, (void *)conn);
    
	if (ctx.lbs_cb.sampling_interval_get_cb) 
    {
        /* Call the application callback function to update the get the current value of the sampling interval */
		ctx.sampling_interval_ms = ctx.lbs_cb.sampling_interval_get_cb(); /* value points to ctx.sampling_interval_ms */
        
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

	if (ctx.lbs_cb.sampling_interval_set_cb) 
    {
		/* Read the received value from the buf parameter. The value is expected to be a 32-bit integer representing the sampling interval in milliseconds. */
		uint32_t val = *((uint32_t *)buf);

		if (val == 0)
		{
			LOG_DBG("Write sampling interval: Incorrect value");
			return BT_GATT_ERR(BT_ATT_ERR_VALUE_NOT_ALLOWED);
		}
		
		/* Call the application callback function to update the sampling interval */
		ctx.lbs_cb.sampling_interval_set_cb(val);
	}

	return len;
}

//------------------------------------------------------------------------------

/* LED Button Service Declaration - Create and add the MY LBS service to the Bluetooth LE stack */
BT_GATT_SERVICE_DEFINE
(   
    env_lb_svc,
    BT_GATT_PRIMARY_SERVICE(BT_UUID_ENV_LBS),
    BT_GATT_CHARACTERISTIC(BT_UUID_ENV_LBS_BUTTON, BT_GATT_CHRC_READ | BT_GATT_CHRC_INDICATE, BT_GATT_PERM_READ, read_button, NULL, &ctx.button_state),
    BT_GATT_CCC(mylbsbc_ccc_cfg_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
    BT_GATT_CHARACTERISTIC(BT_UUID_ENV_LBS_LED, BT_GATT_CHRC_WRITE, BT_GATT_PERM_WRITE_AUTHEN, NULL, write_led, NULL),
    BT_GATT_CHARACTERISTIC(BT_UUID_ENV_LBS_MYSENSOR, BT_GATT_CHRC_NOTIFY, BT_GATT_PERM_NONE, NULL, NULL, NULL),
    BT_GATT_CCC(mylbsbc_ccc_mysensor_cfg_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE), 
	BT_GATT_CHARACTERISTIC(BT_UUID_ENV_LBS_SAMPLING_INTERVAL, BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE_AUTHEN, read_sampling_interval, write_sampling_interval, &ctx.sampling_interval_ms),
);

//------------------------------------------------------------------------------

/* A function to register application callbacks for the LED and Button characteristics  */
int env_lb_service_init(struct env_lb_service_cb *callbacks)
{
	if (callbacks) 
    {
		ctx.lbs_cb.led_set_cb = callbacks->led_set_cb;
		ctx.lbs_cb.button_get_cb = callbacks->button_get_cb;
		ctx.lbs_cb.sampling_interval_get_cb = callbacks->sampling_interval_get_cb;
		ctx.lbs_cb.sampling_interval_set_cb = callbacks->sampling_interval_set_cb;
	}

	return 0;
}

int env_lb_service_send_button_state_indicate(bool button_state)
{
	if (!ctx.indicate_button_enabled) 
		return -EACCES;

    ctx.ind_params.attr = &env_lb_svc.attrs[2];
	ctx.ind_params.func = indicate_ack_cb;
	ctx.ind_params.destroy = NULL;
	ctx.ind_params.data = &button_state;
	ctx.ind_params.len = sizeof(button_state);
	return bt_gatt_indicate(NULL, &ctx.ind_params);
}

int env_lb_service_send_sensor_notify(uint32_t sensor_value)
{
	if (!ctx.notify_mysensor_enabled) 
		return -EACCES;

	return bt_gatt_notify(NULL, &env_lb_svc.attrs[7], &sensor_value, sizeof(sensor_value));
}

//------------------------------------------------------------------------------
