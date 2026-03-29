//------------------------------------------------------------------------------

/// @file battery_service.c
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

#include <zephyr/bluetooth/services/bas.h>

#include <zephyr/logging/log.h>

#include "battery_service.h"

//------------------------------------------------------------------------------

int battery_service_send_battery_notify(uint8_t battery_level)
{
	return bt_bas_set_battery_level(battery_level);
}

//------------------------------------------------------------------------------
