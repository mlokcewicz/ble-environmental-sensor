//------------------------------------------------------------------------------

/// @file env_manager.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include "env_manager.h"

#include <zephyr/logging/log.h>

#include <zephyr/drivers/sensor.h>

//------------------------------------------------------------------------------

LOG_MODULE_REGISTER(env_manager);

const struct device *bme280_dev = DEVICE_DT_GET(DT_NODELABEL(bme280));

//------------------------------------------------------------------------------

int env_manager_init(void)
{
    int ret = device_is_ready(bme280_dev);
	if (!ret)
	{
		LOG_INF("Error: SPI device is not ready, err: %d", ret);
		return ret;
	}

    return 0;
}

int env_manager_process(void)
{
    struct sensor_value temp_val;
    struct sensor_value press_val;
    struct sensor_value hum_val;

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


    // LOG_INF("Compensated temperature value: %d", temp_val.val1);
    // LOG_INF("Compensated humidity value: %d", hum_val.val1);
    // LOG_INF("Compensated pressure value: %d", press_val.val1);

    int thp_service_send_sensor_notify(int, uint32_t sensor_value);
    thp_service_send_sensor_notify(0, temp_val.val1);
    thp_service_send_sensor_notify(1, hum_val.val1); 
    thp_service_send_sensor_notify(2, press_val.val1);

    k_msleep(1000); // Add sampling interval

    return 0;
}

//------------------------------------------------------------------------------
