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

#ifdef CONFIG_MYFUNCTION
#include "my_function.h"
#endif

//------------------------------------------------------------------------------

LOG_MODULE_REGISTER(main);

#define SLEEP_TIME_MS 500

#define UART_RECEIVE_TIMEOUT 	100
#define UART_RECEIVE_BUFF_SIZE 	10

#define BLINK_PERIOD_MS_MAX  1000U

#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)
#define SW0_NODE DT_ALIAS(sw0)

//------------------------------------------------------------------------------

static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec sw0 = GPIO_DT_SPEC_GET(SW0_NODE, gpios);

static struct gpio_callback pin_cb_data;

const struct device *uart = DEVICE_DT_GET(DT_NODELABEL(uart0));

static uint8_t rx_buf[UART_RECEIVE_BUFF_SIZE];

const struct device *dev = DEVICE_DT_GET(DT_NODELABEL(bme280));
const struct device * blink = DEVICE_DT_GET(DT_NODELABEL(blink_led));

//------------------------------------------------------------------------------

void pin_isr(const struct device *dev, struct gpio_callback *cb, gpio_port_pins_t pins)
{
	gpio_pin_toggle_dt(&led1);
	LOG_WRN("Button pressed");
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
			else if (evt->data.rx.buf[evt->data.rx.offset] == '2')
				gpio_pin_toggle_dt(&led1);
		}
		break;
	case UART_RX_DISABLED:
		uart_rx_enable(dev, rx_buf, sizeof rx_buf, UART_RECEIVE_TIMEOUT);
		break;
	default:
		break;
	}
}

//------------------------------------------------------------------------------

typedef struct 
{
    uint32_t x_reading;
    uint32_t y_reading;
    uint32_t z_reading;
} SensorReading;

K_MSGQ_DEFINE(device_message_queue, sizeof(SensorReading), 16, 4);

#define STACKSIZE		 			2048
#define PRODUCER_THREAD_PRIORITY 	6
#define CONSUMER_THREAD_PRIORITY 	7
#define PRODUCER_SLEEP_TIME_MS 	2200

static void producer_func(void *unused1, void *unused2, void *unused3)
{
	ARG_UNUSED(unused1);
	ARG_UNUSED(unused2);
	ARG_UNUSED(unused3);

	while (1) 
	{
		static SensorReading acc_val = {100, 100, 100};

        int ret = k_msgq_put(&device_message_queue,&acc_val,K_FOREVER);
        if (ret)
		{
            LOG_ERR("Return value from k_msgq_put = %d",ret);
        }

		acc_val.x_reading += 1;
		acc_val.y_reading += 1;
		acc_val.z_reading += 1;

		k_msleep(PRODUCER_SLEEP_TIME_MS);
	}
}

static void consumer_func(void *unused1, void *unused2, void *unused3)
{
	ARG_UNUSED(unused1);
	ARG_UNUSED(unused2);
	ARG_UNUSED(unused3);

	while (1) 
	{
		SensorReading temp;
		
		int ret = k_msgq_get(&device_message_queue,&temp,K_FOREVER);
		if (ret)
		{
            LOG_ERR("Return value from k_msgq_get = %d", ret);
        }

		LOG_INF("Values got from the queue: %d.%d.%d\r\n", temp.x_reading, temp.y_reading, temp.z_reading);
	}
}

K_THREAD_DEFINE(producer, STACKSIZE, producer_func, NULL, NULL, NULL, PRODUCER_THREAD_PRIORITY, 0, 0);
K_THREAD_DEFINE(consumer, STACKSIZE, consumer_func, NULL, NULL, NULL, CONSUMER_THREAD_PRIORITY, 0, 0);

//------------------------------------------------------------------------------

int main(void)
{
	LOG_INF("Main started");

	int ret = 0;

	if (!device_is_ready(led0.port))
		return -1;

	if (!device_is_ready(led0.port))
		return -1;

	if (!device_is_ready(sw0.port))
		return -1;

	ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE);
	ret = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE);
	ret = gpio_pin_configure_dt(&sw0, GPIO_INPUT);	

	ret = gpio_pin_interrupt_configure_dt(&sw0, GPIO_INT_EDGE_TO_ACTIVE);

	gpio_init_callback(&pin_cb_data, pin_isr, BIT(sw0.pin));
	gpio_add_callback(sw0.port, &pin_cb_data);

	if (!device_is_ready(uart))
	{
		printk("UART device not ready\r\n");
		return 1;
	}

	ret = uart_callback_set(uart, uart_cb, NULL);
	if (ret)
		return 1;

	ret = uart_tx(uart, "Press 1 or 2 to toggle LEDs\r\n", 30, SYS_FOREVER_US);
	if (ret) 
		return 1;

	ret = uart_rx_enable(uart, rx_buf, sizeof(rx_buf), UART_RECEIVE_TIMEOUT);
	if (ret) 
		return 1;

	// ret = device_is_ready(dev);
	// if (!ret)
	// {
	// 	LOG_INF("Error: SPI device is not ready, err: %d", ret);
	// 	return 0;
	// }

	ret = device_is_ready(blink);
	if (!ret)
	{
		LOG_INF("Error: Blink device is not ready, err: %d", ret);
		return 0;
	}

	ret = blink_off(blink);
	if (ret < 0)
	{
		LOG_ERR("Could not turn off LED (%d)", ret);
		return 0;
	}

	LOG_INF("Setting LED period to %u ms\n", BLINK_PERIOD_MS_MAX);
	blink_set_period_ms(blink, BLINK_PERIOD_MS_MAX);

	while (1)
	{
		ret = gpio_pin_toggle_dt(&led0);

		if (ret < 0)
			return -1;

		k_msleep(SLEEP_TIME_MS);

#ifdef CONFIG_MYFUNCTION
		int a = 3, b = 4;
		printk("The sum of %d and %d is %d\n", a, b, sum(a, b));
#else
		printk("MYFUNCTION not enabled\n");
#endif

#if 0
		struct sensor_value temp_val;
		struct sensor_value press_val;
		struct sensor_value hum_val;

		ret = sensor_sample_fetch(dev);
		if (ret < 0)
		{
			LOG_ERR("Could not fetch sample (%d)", ret);
			return 0;
		}

		if (sensor_channel_get(dev, SENSOR_CHAN_AMBIENT_TEMP, &temp_val))
		{
			LOG_ERR("Could not get sample");
			return 0;
		}

		if (sensor_channel_get(dev, SENSOR_CHAN_PRESS, &press_val))
		{
			LOG_ERR("Could not get sample");
			return 0;
		}

		if (sensor_channel_get(dev, SENSOR_CHAN_HUMIDITY, &hum_val))
		{
			LOG_ERR("Could not get sample");
			return 0;
		}

		LOG_INF("Compensated temperature value: %d", temp_val.val1);
		LOG_INF("Compensated pressure value: %d", press_val.val1);
		LOG_INF("Compensated humidity value: %d", hum_val.val1);
#endif 
	}
}
