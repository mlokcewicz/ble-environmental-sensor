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

//------------------------------------------------------------------------------

LOG_MODULE_REGISTER(main);

#define SLEEP_TIME_MS 500

#define UART_RECEIVE_TIMEOUT 	100
#define UART_RECEIVE_BUFF_SIZE 	10

#define BLINK_PERIOD_MS_MAX  1000U

#define LED0_NODE DT_ALIAS(led0)
#define LED2_NODE DT_ALIAS(led2)
#define SW0_NODE DT_ALIAS(sw0)

//------------------------------------------------------------------------------

static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);
static const struct gpio_dt_spec sw0 = GPIO_DT_SPEC_GET(SW0_NODE, gpios);

static struct gpio_callback pin_cb_data;

const struct device *uart = DEVICE_DT_GET(DT_NODELABEL(uart0));
static uint8_t rx_buf[UART_RECEIVE_BUFF_SIZE];

const struct device *bme280_dev = DEVICE_DT_GET(DT_NODELABEL(bme280));
const struct device *blink = DEVICE_DT_GET(DT_NODELABEL(blink_led));

//------------------------------------------------------------------------------

void pin_isr(const struct device *dev, struct gpio_callback *cb, gpio_port_pins_t pins)
{
	bool pressed = gpio_pin_get_dt(&sw0);
	ble_manager_notify_button_pressed(pressed);

	if (pressed)
	{
		LOG_WRN("Button pressed");

		LOG_INF("Setting LED period to %u ms\n", BLINK_PERIOD_MS_MAX);
		blink_set_period_ms(blink, 100);
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

	if (!device_is_ready(sw0.port))
		return -1;

	ret = gpio_pin_configure_dt(&sw0, GPIO_INPUT);
	if (ret < 0)
		return ret;

	ret = gpio_pin_interrupt_configure_dt(&sw0, GPIO_INT_EDGE_BOTH);
	if (ret < 0)
		return ret;

	gpio_init_callback(&pin_cb_data, pin_isr, BIT(sw0.pin));

	ret = gpio_add_callback(sw0.port, &pin_cb_data);
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

	// ret = device_is_ready(bme280_dev);rj.
	// if (!ret)
	// {
	// 	LOG_INF("Error: SPI device is not ready, err: %d", ret);
	// 	return ret;
	// }

	ret = ble_manager_init(ble_connection_state_cb);
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

		k_msleep(SLEEP_TIME_MS);

#if 0
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
