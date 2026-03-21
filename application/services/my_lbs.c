//------------------------------------------------------------------------------

/// @file my_lbs.c
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

#include "my_lbs.h"

//------------------------------------------------------------------------------

struct my_lbs_ctx 
{
    bool button_state;
    struct my_lbs_cb lbs_cb;
};

//------------------------------------------------------------------------------

LOG_MODULE_DECLARE(my_lbs);

static struct my_lbs_ctx ctx;

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

//------------------------------------------------------------------------------

/* LED Button Service Declaration - Create and add the MY LBS service to the Bluetooth LE stack */
BT_GATT_SERVICE_DEFINE(my_lbs_svc, 
                       BT_GATT_PRIMARY_SERVICE(BT_UUID_LBS),
                       BT_GATT_CHARACTERISTIC(BT_UUID_LBS_BUTTON, BT_GATT_CHRC_READ, BT_GATT_PERM_READ, read_button, NULL, &ctx.button_state),
                       BT_GATT_CHARACTERISTIC(BT_UUID_LBS_LED, BT_GATT_CHRC_WRITE, BT_GATT_PERM_WRITE, NULL, write_led, NULL),
);

//------------------------------------------------------------------------------

/* A function to register application callbacks for the LED and Button characteristics  */
int my_lbs_init(struct my_lbs_cb *callbacks)
{
	if (callbacks) 
    {
		ctx.lbs_cb.led_set_cb = callbacks->led_set_cb;
		ctx.lbs_cb.button_get_cb = callbacks->button_get_cb;
	}

	return 0;
}

//------------------------------------------------------------------------------
