//------------------------------------------------------------------------------

/// @file ui_manager.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include "ui_manager.h"

#include <app_event.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>

#include <blink.h>

//------------------------------------------------------------------------------

#define BLINK_PERIOD_MS_SLOW  1000U
#define BLINK_PERIOD_MS_FAST  100U

#define LED1_NODE DT_ALIAS(led1) // Indicates Bluetooth connection state (on when connected, off when disconnected)
#define LED2_NODE DT_ALIAS(led2) // Used by LBS service
#define SW0_NODE DT_ALIAS(sw0)   // Used by LBS service
#define SW1_NODE DT_ALIAS(sw1)   // Used to unpair peer
#define SW2_NODE DT_ALIAS(sw2)   // Used to enter pairing mode

//------------------------------------------------------------------------------

LOG_MODULE_REGISTER(ui_manager);

ZBUS_MSG_SUBSCRIBER_DEFINE(ui_manager_sub); 

static const struct gpio_dt_spec ble_conn_led = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec lbs_led = GPIO_DT_SPEC_GET(LED2_NODE, gpios);
static const struct gpio_dt_spec lbs_sw = GPIO_DT_SPEC_GET(SW0_NODE, gpios);
static const struct gpio_dt_spec ble_unpair_sw = GPIO_DT_SPEC_GET(SW1_NODE, gpios);
static const struct gpio_dt_spec ble_pairing_mode_sw = GPIO_DT_SPEC_GET(SW2_NODE, gpios);

static struct gpio_callback pin_cb_data;

const struct device *pairing_blink_led = DEVICE_DT_GET(DT_NODELABEL(blink_led));

static bool button_state = false;

//------------------------------------------------------------------------------

void pin_isr(const struct device *dev, struct gpio_callback *cb, gpio_port_pins_t pins)
{
	struct app_event event;

	if (pins & BIT(lbs_sw.pin))
	{
		LOG_DBG("SW0 interrupt triggered");
		
		bool pressed = gpio_pin_get_dt(&lbs_sw);
		button_state = pressed;

		event.type = APP_EVENT_LBS_BUTTON_STATE_CHANGED;
		event.button_state = pressed;
		int ret = zbus_chan_pub(&ble_control_chan, &event, K_MSEC(10));
		if (ret < 0)
		{
			LOG_ERR("Failed to publish LBS button state change event (err %d)", ret);
		}
	}	

	if (pins & BIT(ble_unpair_sw.pin))
	{
		LOG_DBG("SW1 interrupt triggered");

		bool pressed = gpio_pin_get_dt(&ble_unpair_sw);
		if (pressed)
		{
			LOG_INF("Unpairing Bluetooth peer");

			event.type = APP_EVENT_BLE_UNPAIR_REQ;
			int ret = zbus_chan_pub(&ble_control_chan, &event, K_MSEC(10));
			if (ret < 0)			
			{
				LOG_ERR("Failed to publish BLE unpairing request event (err %d)", ret);
			}
		}
	}

	if (pins & BIT(ble_pairing_mode_sw.pin))
	{
		LOG_DBG("SW2 interrupt triggered");

		bool pressed = gpio_pin_get_dt(&ble_pairing_mode_sw);
		if (pressed)
		{
			LOG_INF("Entering pairing mode");

			blink_set_period_ms(pairing_blink_led, BLINK_PERIOD_MS_FAST);
			event.type = APP_EVENT_BLE_PAIRING_MODE_REQ;
			int ret = zbus_chan_pub(&ble_control_chan, &event, K_MSEC(10));
			if (ret < 0)	
			{
				LOG_ERR("Failed to publish BLE pairing mode request event (err %d)", ret);
			}
		}
	}
}

static void set_conn_state_led(bool connected)
{
	if (connected)
		blink_set_period_ms(pairing_blink_led, BLINK_PERIOD_MS_SLOW);

	gpio_pin_set_dt(&ble_conn_led, connected);
}

//------------------------------------------------------------------------------

int ui_manager_init(void)
{
    int ret = 0;

	if (!device_is_ready(ble_conn_led.port))
		return -1;

	ret = gpio_pin_configure_dt(&ble_conn_led, GPIO_OUTPUT_INACTIVE);
	if (ret < 0)
		return ret;

	if (!device_is_ready(lbs_led.port))
		return -1;

	ret = gpio_pin_configure_dt(&lbs_led, GPIO_OUTPUT_INACTIVE);
	if (ret < 0)
		return ret;

	if (!device_is_ready(lbs_sw.port))
		return -1;

	ret = gpio_pin_configure_dt(&lbs_sw, GPIO_INPUT);
	if (ret < 0)
		return ret;

	ret = gpio_pin_interrupt_configure_dt(&lbs_sw, GPIO_INT_EDGE_BOTH);
	if (ret < 0)
		return ret;

	if (!device_is_ready(ble_unpair_sw.port))
		return -1;

	ret = gpio_pin_configure_dt(&ble_unpair_sw, GPIO_INPUT);
	if (ret < 0)
		return ret;

	ret = gpio_pin_interrupt_configure_dt(&ble_unpair_sw, GPIO_INT_EDGE_BOTH);
	if (ret < 0)
		return ret;

	if (!device_is_ready(ble_pairing_mode_sw.port))
		return -1;

	ret = gpio_pin_configure_dt(&ble_pairing_mode_sw, GPIO_INPUT);
	if (ret < 0)
		return ret;

	ret = gpio_pin_interrupt_configure_dt(&ble_pairing_mode_sw, GPIO_INT_EDGE_BOTH);
	if (ret < 0)
		return ret;

	gpio_init_callback(&pin_cb_data, pin_isr, BIT(lbs_sw.pin) | BIT(ble_unpair_sw.pin) | BIT(ble_pairing_mode_sw.pin));

	ret = gpio_add_callback(lbs_sw.port, &pin_cb_data);
	if (ret < 0)
		return ret;

	ret = gpio_add_callback(ble_unpair_sw.port, &pin_cb_data);
	if (ret < 0)
		return ret;

	ret = gpio_add_callback(ble_unpair_sw.port, &pin_cb_data);
	if (ret < 0)
		return ret;

	ret = gpio_add_callback(ble_pairing_mode_sw.port, &pin_cb_data);
	if (ret < 0)
		return ret;
    
	ret = device_is_ready(pairing_blink_led);
	if (!ret)
	{
		LOG_ERR("Error: Blink device is not ready, err: %d", ret);
		return ret;
	}

    ret = blink_set_period_ms(pairing_blink_led, BLINK_PERIOD_MS_SLOW);
	if (ret < 0)
	{
		LOG_ERR("Could set LED blink (%d)", ret);
		return ret;
	}

    return 0;
}

int ui_manager_process(void)
{
	const struct zbus_channel *chan;
    struct app_event event;

    while (1)
    {
		int ret = zbus_sub_wait_msg(&ui_manager_sub, &chan, &event, K_FOREVER);
		if (ret != 0)
			continue;

		if (chan == &ui_control_chan)
		{
			switch (event.type)
			{
			case APP_EVENT_BLE_CONNECTION_STATE_CHANGED:
				set_conn_state_led(event.ble_connected);
				break;

			default:
				break;
			}
		}
	}
}

//------------------------------------------------------------------------------
