/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/util_macro.h>

#include <zephyr/logging/log.h>

#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/sensor.h>

#include <blink.h>

#include "ble_manager.h"
#include "services/env_lb_service.h"

//------------------------------------------------------------------------------

LOG_MODULE_REGISTER(main);

#define SLEEP_TIME_MS 1000

#define UART_RECEIVE_TIMEOUT 	100
#define UART_RECEIVE_BUFF_SIZE 	10

#define BLINK_PERIOD_MS_MAX  1000U

#define LED0_NODE DT_ALIAS(led0) // Blink in while loop
#define LED2_NODE DT_ALIAS(led2) // Indicates Bluetooth connection state (on when connected, off when disconnected)
#define LED3_NODE DT_ALIAS(led3) // Used by LBS service
#define SW0_NODE DT_ALIAS(sw0)   // Used by LBS service
#define SW1_NODE DT_ALIAS(sw1)   // Used to unpair peer
#define SW2_NODE DT_ALIAS(sw2)   // Used to enter pairing mode

//------------------------------------------------------------------------------

static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);
static const struct gpio_dt_spec led3 = GPIO_DT_SPEC_GET(LED3_NODE, gpios);
static const struct gpio_dt_spec sw0 = GPIO_DT_SPEC_GET(SW0_NODE, gpios);
static const struct gpio_dt_spec sw1 = GPIO_DT_SPEC_GET(SW1_NODE, gpios);
static const struct gpio_dt_spec sw2 = GPIO_DT_SPEC_GET(SW2_NODE, gpios);

static struct gpio_callback pin_cb_data;

const struct device *uart = DEVICE_DT_GET(DT_NODELABEL(uart0));
static uint8_t rx_buf[UART_RECEIVE_BUFF_SIZE];

const struct device *bme280_dev = DEVICE_DT_GET(DT_NODELABEL(bme280));
const struct device *blink = DEVICE_DT_GET(DT_NODELABEL(blink_led));

static bool button_state = false;

//------------------------------------------------------------------------------

void pin_isr(const struct device *dev, struct gpio_callback *cb, gpio_port_pins_t pins)
{
	if (pins & BIT(sw0.pin))
	{
		LOG_DBG("SW0 interrupt triggered");
		
		bool pressed = gpio_pin_get_dt(&sw0);
		button_state = pressed;

		env_lb_service_send_button_state_indicate(pressed);

		LOG_INF("Setting LED period to %u ms\n", BLINK_PERIOD_MS_MAX);
		blink_set_period_ms(blink, 100);
	}	

	if (pins & BIT(sw1.pin))
	{
		LOG_DBG("SW1 interrupt triggered");

		bool pressed = gpio_pin_get_dt(&sw1);
		if (pressed)
		{
			LOG_INF("Unpairing Bluetooth peer");
			ble_manager_unpair();
		}
	}

	if (pins & BIT(sw2.pin))
	{
		LOG_DBG("SW2 interrupt triggered");

		bool pressed = gpio_pin_get_dt(&sw2);
		if (pressed)
		{
			LOG_INF("Entering pairing mode");
			ble_manager_enter_pairing_mode();
		}
	}
}

static void uart_cb(const struct device *dev, struct uart_event *evt, void *user_data)
{
	switch (evt->type)
	{
	case UART_RX_RDY:
		if ((evt->data.rx.len) == 1)
		{
			if (evt->data.rx.buf[evt->data.rx.offset] == '1')
				gpio_pin_toggle_dt(&led0);
		}
		break;
	case UART_RX_DISABLED:
		uart_rx_enable(dev, rx_buf, sizeof rx_buf, UART_RECEIVE_TIMEOUT);
		break;
	default:
		break;
	}
}

static void ble_connection_state_cb(bool connected)
{
	gpio_pin_set_dt(&led2, connected);
}

static bool app_button_cb(void)
{
	return button_state;
}

static void app_led_cb(const bool led_state)
{
	gpio_pin_set_dt(&led3, led_state);
}

//------------------------------------------------------------------------------

int main(void)
{
	LOG_INF("Main started");

	int ret = 0;

	if (!device_is_ready(led0.port))
		return -1;

	ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_INACTIVE);
	if (ret < 0)
		return ret;

	if (!device_is_ready(led2.port))
		return -1;

	ret = gpio_pin_configure_dt(&led2, GPIO_OUTPUT_INACTIVE);
	if (ret < 0)
		return ret;

	if (!device_is_ready(led3.port))
		return -1;

	ret = gpio_pin_configure_dt(&led3, GPIO_OUTPUT_INACTIVE);
	if (ret < 0)
		return ret;

	if (!device_is_ready(sw0.port))
		return -1;

	ret = gpio_pin_configure_dt(&sw0, GPIO_INPUT);
	if (ret < 0)
		return ret;

	ret = gpio_pin_interrupt_configure_dt(&sw0, GPIO_INT_EDGE_BOTH);
	if (ret < 0)
		return ret;

	if (!device_is_ready(sw1.port))
		return -1;

	ret = gpio_pin_configure_dt(&sw1, GPIO_INPUT);
	if (ret < 0)
		return ret;

	ret = gpio_pin_interrupt_configure_dt(&sw1, GPIO_INT_EDGE_BOTH);
	if (ret < 0)
		return ret;

	if (!device_is_ready(sw2.port))
		return -1;

	ret = gpio_pin_configure_dt(&sw2, GPIO_INPUT);
	if (ret < 0)
		return ret;

	ret = gpio_pin_interrupt_configure_dt(&sw2, GPIO_INT_EDGE_BOTH);
	if (ret < 0)
		return ret;

	gpio_init_callback(&pin_cb_data, pin_isr, BIT(sw0.pin) | BIT(sw1.pin) | BIT(sw2.pin));

	ret = gpio_add_callback(sw0.port, &pin_cb_data);
	if (ret < 0)
		return ret;

	ret = gpio_add_callback(sw1.port, &pin_cb_data);
	if (ret < 0)
		return ret;

	ret = gpio_add_callback(sw1.port, &pin_cb_data);
	if (ret < 0)
		return ret;

	ret = gpio_add_callback(sw2.port, &pin_cb_data);
	if (ret < 0)
		return ret;

	// if (!device_is_ready(uart))
	// 	return -1;

	// ret = uart_callback_set(uart, uart_cb, NULL);
	// if (ret < 0)
	// 	return ret;

	// ret = uart_tx(uart, "Press 1 or 2 to toggle LEDs\r\n", 30, SYS_FOREVER_US);
	// if (ret < 0)
	// 	return ret;

	// ret = uart_rx_enable(uart, rx_buf, sizeof(rx_buf), UART_RECEIVE_TIMEOUT);
	// if (ret < 0)
	// 	return ret;

	ret = device_is_ready(blink);
	if (!ret)
	{
		LOG_ERR("Error: Blink device is not ready, err: %d", ret);
		return ret;
	}

	ret = blink_off(blink);
	if (ret < 0)
	{
		LOG_ERR("Could not turn off LED (%d)", ret);
		return ret;
	}

	ret = device_is_ready(bme280_dev);
	if (!ret)
	{
		LOG_INF("Error: SPI device is not ready, err: %d", ret);
		return ret;
	}

	struct ble_manager_cfg ble_manager_cfg = 
	{
		.connection_state_cb = ble_connection_state_cb,
		.led_set_cb = app_led_cb,
		.button_get_cb = app_button_cb,
	};

	ret = ble_manager_init(&ble_manager_cfg);
	if (ret < 0)
	{
		LOG_ERR("Bluetooth initialization failed (err %d)", ret);
		return ret;
	}

	while (1)
	{
		ret = gpio_pin_toggle_dt(&led0);

		if (ret < 0)
			return -1;

		env_lb_service_send_sensor_notify(k_uptime_get_32() / 1000); // Send uptime in seconds as sensor value

		k_msleep(SLEEP_TIME_MS);

#if 1
		struct sensor_value temp_val;
		struct sensor_value press_val;
		struct sensor_value hum_val;

		ret = sensor_sample_fetch(bme280_dev);
		if (ret < 0)
		{
			LOG_ERR("Could not fetch sample (%d)", ret);
			return ret;
		}

		if (sensor_channel_get(bme280_dev, SENSOR_CHAN_AMBIENT_TEMP, &temp_val))
		{
			LOG_ERR("Could not get sample");
			return -1;
		}

		if (sensor_channel_get(bme280_dev, SENSOR_CHAN_PRESS, &press_val))
		{
			LOG_ERR("Could not get sample");
			return -1;
		}

		if (sensor_channel_get(bme280_dev, SENSOR_CHAN_HUMIDITY, &hum_val))
		{
			LOG_ERR("Could not get sample");
			return -1;
		}

		LOG_INF("Compensated temperature value: %d", temp_val.val1);
		LOG_INF("Compensated pressure value: %d", press_val.val1);
		LOG_INF("Compensated humidity value: %d", hum_val.val1);
#endif 
	}
}
