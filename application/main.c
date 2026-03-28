/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/util_macro.h>

#include <zephyr/drivers/watchdog.h>
#include <zephyr/logging/log.h>

#include <ble_manager.h>
#include <ui_manager.h>
#include <env_manager.h>

//------------------------------------------------------------------------------

LOG_MODULE_REGISTER(main);

#define SLEEP_TIME_MS 1000
#define WDT_TIMEOUT_MS 2000

//------------------------------------------------------------------------------
/* Threads definitions */

#define UI_MANAGER_THREAD_STACK_SIZE 2048
#define ENV_MANAGER_THREAD_STACK_SIZE 2048

#define ENV_MANAGER_THREAD_PRIORITY 5
#define UI_MANAGER_THREAD_PRIORITY 5

K_THREAD_STACK_DEFINE(ui_manager_stack_area, UI_MANAGER_THREAD_STACK_SIZE);
K_THREAD_STACK_DEFINE(env_manager_stack_area, ENV_MANAGER_THREAD_STACK_SIZE);

static struct k_thread ui_manager_thread_data;
static struct k_thread env_manager_thread_data;

static void ui_manager_thread_func(void *unused1, void *unused2, void *unused3)
{
	LOG_INF("UI Manager thread started");

	while (1)
	{
		ui_manager_process();
	}
}

static void env_manager_thread_func(void *unused1, void *unused2, void *unused3)
{
	LOG_INF("Environment Manager thread started");

	while (1)
	{
		env_manager_process();
	}
}

//------------------------------------------------------------------------------

const struct device *const wdt = DEVICE_DT_GET(DT_ALIAS(watchdog0));

//------------------------------------------------------------------------------

static int configure_watchdog(void)
{
	if (!device_is_ready(wdt))
	{
		LOG_ERR("Watchdog device is not ready");
		return -ENODEV;
	}

	struct wdt_timeout_cfg wdt_config = 
	{
		.window.min = 0,
		.window.max = WDT_TIMEOUT_MS,
		.callback = NULL, // No callback, just reset
		.flags = WDT_FLAG_RESET_SOC,
	};

	int wdt_channel_id = wdt_install_timeout(wdt, &wdt_config);
	if (wdt_channel_id < 0)
	{
		LOG_ERR("Failed to install watchdog timeout (err %d)", wdt_channel_id);
		return wdt_channel_id;
	}

	int ret = wdt_setup(wdt, WDT_OPT_PAUSE_HALTED_BY_DBG);
	if (ret < 0)
	{
		LOG_ERR("Failed to setup watchdog (err %d)", ret);
		return ret;
	}

	return wdt_channel_id;
}	

static void app_ble_connection_state_cb(bool connected)
{
	// gpio_pin_set_dt(&led2, connected);
	void remove_me_set_ui_conn(bool connected);
	remove_me_set_ui_conn(connected);
}

static bool app_lbs_button_cb(void)
{
	// return button_state;
	return false;
}

static void app_lbs_led_cb(const bool led_state)
{
	// gpio_pin_set_dt(&led3, led_state);
}

//------------------------------------------------------------------------------

int main(void)
{
	LOG_INF("Main started");

	int wdt_channel_id = configure_watchdog();
	if (wdt_channel_id < 0)
	{
		LOG_ERR("Watchdog configuration failed, err: %d", wdt_channel_id);
		return wdt_channel_id;
	}

	int ret = ui_manager_init();
	if (ret < 0)
	{
		LOG_ERR("UI Manager initialization failed: %d", ret);
		return ret;
	}
	
	ret = env_manager_init();
	if (ret < 0)
	{
		LOG_ERR("Environment Manager initialization failed: %d", ret);
		return ret;
	}

	struct ble_manager_cfg ble_manager_cfg = 
	{
		.connection_state_cb = app_ble_connection_state_cb,
		.led_set_cb = app_lbs_led_cb,
		.button_get_cb = app_lbs_button_cb,
	};

	ret = ble_manager_init(&ble_manager_cfg);
	if (ret < 0)
	{
		LOG_ERR("BLE Manager initialization failed (err %d)", ret);
		return ret;
	}

	k_tid_t my_tid = k_thread_create(&ui_manager_thread_data, ui_manager_stack_area,
                                 K_THREAD_STACK_SIZEOF(ui_manager_stack_area),
                                 ui_manager_thread_func,
                                 NULL, NULL, NULL,
                                 UI_MANAGER_THREAD_PRIORITY, 0, K_NO_WAIT);

	LOG_INF("UI Manager thread created with ID %d", (int)my_tid);

	k_tid_t env_tid = k_thread_create(&env_manager_thread_data, env_manager_stack_area,
								 K_THREAD_STACK_SIZEOF(env_manager_stack_area),
								 env_manager_thread_func,
								 NULL, NULL, NULL,
								 ENV_MANAGER_THREAD_PRIORITY, 0, K_NO_WAIT);

	LOG_INF("Environment Manager thread created with ID %d", (int)env_tid);

	while (1)
	{


		int env_lb_service_send_sensor_notify(uint32_t sensor_value);
		env_lb_service_send_sensor_notify(k_uptime_get_32() / 1000); // Send uptime in seconds as sensor value



		wdt_feed(wdt, wdt_channel_id);

		k_msleep(SLEEP_TIME_MS);
	}
}
