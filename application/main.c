/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/util_macro.h>

#include <zephyr/drivers/watchdog.h>
#include <zephyr/logging/log.h>

#include <zephyr/drivers/adc.h>

#include <system_stats.h>

#include <app_event.h>

#include <ble_manager.h>
#include <ui_manager.h>
#include <env_manager.h>

//------------------------------------------------------------------------------

LOG_MODULE_REGISTER(main);

#define SLEEP_TIME_MS 1000
#define WDT_TIMEOUT_MS 2000
#define COIN_CELL_MIN_VOLTAGE_MV 2000
#define COIN_CELL_MAX_VOLTAGE_MV 3200

//------------------------------------------------------------------------------
/* Threads definitions */

#define BLE_MANAGER_THREAD_STACK_SIZE 1024
#define UI_MANAGER_THREAD_STACK_SIZE 1024
#define ENV_MANAGER_THREAD_STACK_SIZE 1024

#define BLE_MANAGER_THREAD_PRIORITY 5
#define ENV_MANAGER_THREAD_PRIORITY 5
#define UI_MANAGER_THREAD_PRIORITY 5

K_THREAD_STACK_DEFINE(ble_manager_stack_area, BLE_MANAGER_THREAD_STACK_SIZE);
K_THREAD_STACK_DEFINE(ui_manager_stack_area, UI_MANAGER_THREAD_STACK_SIZE);
K_THREAD_STACK_DEFINE(env_manager_stack_area, ENV_MANAGER_THREAD_STACK_SIZE);

static struct k_thread ble_manager_thread_data;
static struct k_thread ui_manager_thread_data;
static struct k_thread env_manager_thread_data;

static void ble_manager_thread_func(void *unused1, void *unused2, void *unused3)
{
	LOG_INF("BLE Manager thread started");

	while (1)
	{
		ble_manager_process();
	}
}

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

static const struct device *const wdt = DEVICE_DT_GET(DT_ALIAS(watchdog0));
static const struct adc_dt_spec adc_channel = ADC_DT_SPEC_GET(DT_PATH(zephyr_user));

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

static int configure_battery_monitor(struct adc_sequence *adc_sequence)
{
	if (!adc_is_ready_dt(&adc_channel))
	{
		LOG_ERR("ADC controller devivce %s not ready", adc_channel.dev->name);
		return 0;
	}

	int err = adc_channel_setup_dt(&adc_channel);
	if (err < 0)
	{
		LOG_ERR("Could not setup channel #%d (%d)", 0, err);
		return 0;
	}

	err = adc_sequence_init_dt(&adc_channel, adc_sequence);
	if (err < 0)
	{
		LOG_ERR("Could not initalize sequnce");
		return 0;
	}

	return 0;
}

static int read_battery_monitor_level(struct adc_sequence *adc_sequence, int16_t *val)
{
	int err = adc_read(adc_channel.dev, adc_sequence);
	if (err < 0)
	{
		LOG_ERR("Could not read (%d)", err);
		return err;
	}

	int val_mv = (int)*val;
	err = adc_raw_to_millivolts_dt(&adc_channel, &val_mv);
	if (err < 0)
	{
		LOG_WRN(" (value in mV not available)\n");
	}
	else
	{
		LOG_INF(" = %d mV", val_mv);
	}

	uint8_t percentage = 0;

	if (val_mv <= COIN_CELL_MIN_VOLTAGE_MV) percentage = 0;
	else if (val_mv >= COIN_CELL_MAX_VOLTAGE_MV) percentage = 100;
	else percentage = (uint8_t)(((val_mv - COIN_CELL_MIN_VOLTAGE_MV) * 100) / (COIN_CELL_MAX_VOLTAGE_MV - COIN_CELL_MIN_VOLTAGE_MV));

	*val = percentage;

	return 0;
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

	int16_t val = 0;
	struct adc_sequence adc_sequence = {.buffer = &val, .buffer_size = sizeof(val)};
	int ret = configure_battery_monitor(&adc_sequence);
	if (ret < 0)
	{
		LOG_ERR("ADC configuration failed, err: %d", ret);
		return ret;
	}

	ret = ui_manager_init();
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
	
	ret = ble_manager_init();
	if (ret < 0)
	{
		LOG_ERR("BLE Manager initialization failed (err %d)", ret);
		return ret;
	}

	k_tid_t ui_tid = k_thread_create(&ui_manager_thread_data, ui_manager_stack_area,
									 K_THREAD_STACK_SIZEOF(ui_manager_stack_area),
									 ui_manager_thread_func,
									 NULL, NULL, NULL,
									 UI_MANAGER_THREAD_PRIORITY, 0, K_NO_WAIT);
	
	k_thread_name_set(ui_tid, "ui");

	LOG_INF("UI Manager thread created with ID %d", (int)ui_tid);

	k_tid_t env_tid = k_thread_create(&env_manager_thread_data, env_manager_stack_area,
									  K_THREAD_STACK_SIZEOF(env_manager_stack_area),
									  env_manager_thread_func,
									  NULL, NULL, NULL,
									  ENV_MANAGER_THREAD_PRIORITY, 0, K_NO_WAIT);

	k_thread_name_set(env_tid, "env");

	LOG_INF("Environment Manager thread created with ID %d", (int)env_tid);

	k_tid_t ble_tid = k_thread_create(&ble_manager_thread_data, ble_manager_stack_area,
									  K_THREAD_STACK_SIZEOF(ble_manager_stack_area),
									  ble_manager_thread_func,
									  NULL, NULL, NULL,
									  BLE_MANAGER_THREAD_PRIORITY, 0, K_NO_WAIT);

	k_thread_name_set(ble_tid, "ble");

	LOG_INF("BLE Manager thread created with ID %d", (int)ble_tid);
 
	while (1)
	{
		wdt_feed(wdt, wdt_channel_id);

		int ret = read_battery_monitor_level(&adc_sequence, &val);
		if (ret < 0)
		{
			LOG_ERR("ADC read failed, err: %d", ret);
		}
	
		struct app_event event = {.type = APP_EVENT_BATTERY_LEVEL_IND, .battery_level = (uint8_t)val};

		ret = zbus_chan_pub(&ble_control_chan, &event, K_MSEC(10));
		if (ret < 0)
		{
			LOG_ERR("Failed to publish battery level ind event (err %d)", ret);
		}

#ifdef CONFIG_SYSTEM_STATS
		system_stats_print();
#endif

		k_msleep(SLEEP_TIME_MS);
	}
}
