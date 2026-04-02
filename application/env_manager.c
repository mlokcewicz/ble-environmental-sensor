//------------------------------------------------------------------------------

/// @file env_manager.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include "env_manager.h"

#include <app_event.h>

#include <stdint.h> 

#include <zephyr/logging/log.h>
#include <zephyr/settings/settings.h>
#include <zephyr/drivers/sensor.h>

//------------------------------------------------------------------------------

#define SETTINGS_PATH "env/sampling_interval"
#define SAMPLING_INTERVAL_DEFAULT_MS 1000

//------------------------------------------------------------------------------

LOG_MODULE_REGISTER(env_manager);

ZBUS_MSG_SUBSCRIBER_DEFINE(env_manager_sub); 
ZBUS_CHAN_ADD_OBS(env_control_chan, env_manager_sub, 0);

struct env_manager_ctx 
{
    uint32_t sampling_interval_ms;
};

static struct env_manager_ctx ctx = 
{
    .sampling_interval_ms = SAMPLING_INTERVAL_DEFAULT_MS
};

const struct device *bme280_dev = DEVICE_DT_GET(DT_NODELABEL(bme280));

//------------------------------------------------------------------------------

static int settings_set_cb(const char *name, size_t len, settings_read_cb read_cb, void *cb_arg)
{
    if (strcmp(name, "sampling_interval") == 0)
    {
        if (len == sizeof(ctx.sampling_interval_ms))
        {
            read_cb(cb_arg, &ctx.sampling_interval_ms, sizeof(ctx.sampling_interval_ms));
            return 0;
        }
    }
    return -ENOENT;
}

SETTINGS_STATIC_HANDLER_DEFINE(env, "env", NULL, settings_set_cb, NULL, NULL);

static int sampling_interval_set(uint32_t sampling_interval_ms)
{
    ctx.sampling_interval_ms = sampling_interval_ms;

    int ret = settings_save_one(SETTINGS_PATH, &sampling_interval_ms, sizeof(sampling_interval_ms));
    if (ret < 0)
    {
        LOG_ERR("Failed to save sampling interval to settings (err %d)", ret);
        return ret;
    }

    return 0;
}

//------------------------------------------------------------------------------

int env_manager_init(void)
{
    int ret = device_is_ready(bme280_dev);
	if (!ret)
	{
		LOG_INF("Error: SPI device is not ready, err: %d", ret);
		return ret;
	}

    settings_subsys_init();
    settings_load_subtree("env");

    struct app_event event = {.type = APP_EVENT_SAMPLING_INTERVAL_SET_REQ, .sampling_interval_ms = ctx.sampling_interval_ms};
    ret = zbus_chan_pub(&ble_control_chan, &event, K_MSEC(10));
    if (ret < 0)    
    {
        LOG_ERR("Failed to publish initial sampling interval update event (err %d)", ret);
        return ret;
    }

    return 0;
}

int env_manager_sampling_interval_get(uint32_t *sampling_interval_ms)
{
    if (sampling_interval_ms == NULL)
        return -EINVAL;

    *sampling_interval_ms = ctx.sampling_interval_ms;

    return 0;
}

int env_manager_process(void)
{
    struct sensor_value temp_val;
    struct sensor_value press_val;
    struct sensor_value hum_val;

    while (1)
    {
        int ret = sensor_sample_fetch(bme280_dev);
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

        if (sensor_channel_get(bme280_dev, SENSOR_CHAN_HUMIDITY, &hum_val))
        {
            LOG_ERR("Could not get sample");
            return -1;
        }

        if (sensor_channel_get(bme280_dev, SENSOR_CHAN_PRESS, &press_val))
        {
            LOG_ERR("Could not get sample");
            return -1;
        }

        LOG_INF("Compensated temperature value: %d.%d", temp_val.val1, temp_val.val2);
        LOG_INF("Compensated humidity value: %d.%d", hum_val.val1, hum_val.val2);
        LOG_INF("Compensated pressure value: %d.%d", press_val.val1, press_val.val2);

        uint16_t temp_celsius_exp1 = temp_val.val1 * 10 + (temp_val.val2 / 100000);
        uint16_t humidity_percent_exp1 = hum_val.val1 * 10 + (hum_val.val2 / 100000);
        uint16_t pressure_hpa_exp1 = press_val.val1 * 100 + (press_val.val2 / 10000);

        struct app_event event =
        {
            .type = APP_EVENT_SENSOR_DATA_IND,
            .sensor_data =
            {
                .temp_celsius_exp1 = temp_celsius_exp1,
                .humidity_percent_exp1 = humidity_percent_exp1,
                .pressure_hpa_exp1 = pressure_hpa_exp1,
            }
        };

        ret = k_msgq_put(&sensor_event_queue, &event, K_NO_WAIT);
        if (ret < 0)
        {
            LOG_ERR("Failed to put sensor data update event in the queue (err %d)", ret);
        }

        const struct zbus_channel *chan;

        ret = zbus_sub_wait_msg(&env_manager_sub, &chan, &event, K_NO_WAIT);
        if (ret == 0 && chan == &env_control_chan)
        {
            switch (event.type)
            {
            case APP_EVENT_SAMPLING_INTERVAL_SET_REQ:
                sampling_interval_set(event.sampling_interval_ms);
                break;

            default:
                break;
            }
        }

        k_msleep(ctx.sampling_interval_ms);
    }

    return 0;
}

//------------------------------------------------------------------------------
