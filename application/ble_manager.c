//------------------------------------------------------------------------------

/// @file ble_manager.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include "ble_manager.h"

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gap.h>

#include <zephyr/bluetooth/conn.h>

#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/addr.h>

#include <bluetooth/services/lbs.h>

//------------------------------------------------------------------------------

#define DEVICE_NAME "Env_Sensor"
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

#define COMPANY_ID_CODE 0x0059 // Nordic Semiconductor ASA

//------------------------------------------------------------------------------

LOG_MODULE_REGISTER(ble_manager, LOG_LEVEL_INF);

//------------------------------------------------------------------------------

typedef struct adv_mfg_data 
{
	uint16_t company_code; /* Company Identifier Code. */
	uint16_t number_press; /* Number of times Button 1 is pressed */
} adv_mfg_data_t;

//------------------------------------------------------------------------------

// Advertising custom data - manufacturer specific data
static adv_mfg_data_t adv_mfg_data = {COMPANY_ID_CODE, 0x00};

// Advertising parameters
static const struct bt_le_adv_param *adv_param = BT_LE_ADV_PARAM(
    (BT_LE_ADV_OPT_CONN | BT_LE_ADV_OPT_USE_IDENTITY), /* Connectable advertising and use identity address */
	BT_GAP_ADV_FAST_INT_MIN_1, /* 0x30 units, 48 units, 30ms */
	BT_GAP_ADV_FAST_INT_MAX_1, /* 0x60 units, 96 units, 60ms */
	NULL); /* Set to NULL for undirected advertising */


// Advertising data - maximum 31 bytes for legacy advertising
static const struct bt_data ad[] = 
{
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
    BT_DATA(BT_DATA_MANUFACTURER_DATA, (unsigned char *)&adv_mfg_data, sizeof(adv_mfg_data)),
};

static unsigned char url_data[] = {0x17,'/','/','g','i','t','h','u','b','.','c','o','m','/',
                                        'm','l','o','k','c','e','w','i','c','z'}; ;

// Scan response data - maximum 31 bytes for legacy advertising
static const struct bt_data sd[] = 
{
    // BT_DATA(BT_DATA_URI, url_data, sizeof(url_data)),
    BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_128_ENCODE(0x00001523, 0x1212, 0xefde, 0x1523, 0x785feabcd123)), // LBS UUID
};

static struct bt_conn *my_conn = NULL;
static ble_manager_connection_state_cb connection_state_callback = NULL;

//------------------------------------------------------------------------------

static struct k_work update_adv_data_work;

static void update_adv_data_work_handler(struct k_work *work)
{
    adv_mfg_data.number_press++;

    int ret = bt_le_adv_update_data(ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
    if (ret < 0)
    {
        LOG_ERR("Failed to update advertising data (err %d)", ret);
    }

    LOG_INF("Advertising data updated: number_press = %d", adv_mfg_data.number_press);
}

static struct k_work adv_work;

static void adv_work_handler(struct k_work *work)
{
	int ret = bt_le_adv_start(adv_param, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));

	if (ret) 
    {
		LOG_ERR("Advertising failed to start (err %d)", ret);
		return;
	}

	LOG_INF("Advertising successfully started");
}

static void advertising_start(void)
{
	k_work_submit(&adv_work);
}

static void on_connected(struct bt_conn *conn, uint8_t err)
{
    if (err) 
    {
        LOG_ERR("Connection error %d", err);
        return;
    }

    LOG_INF("Connected");
    my_conn = bt_conn_ref(conn);

    if (connection_state_callback)
        connection_state_callback(true);
}

static void on_disconnected(struct bt_conn *conn, uint8_t reason)
{
    LOG_INF("Disconnected. Reason %d", reason);
    bt_conn_unref(my_conn);

    if (connection_state_callback)
        connection_state_callback(false);
}

static void on_recycled(void)
{
    advertising_start();
}

BT_CONN_CB_DEFINE(conn_callbacks) = 
{
	.connected = on_connected,
    .disconnected = on_disconnected,
	.recycled = on_recycled,
};

//------------------------------------------------------------------------------

int ble_manager_init(ble_manager_connection_state_cb connection_state_cb)
{
    int ret;

    k_work_init(&adv_work, adv_work_handler);
    k_work_init(&update_adv_data_work, update_adv_data_work_handler);

    connection_state_callback = connection_state_cb;

    bt_addr_le_t addr;
    ret = bt_addr_le_from_str("FF:EE:DD:CC:BB:AA", "random", &addr);
    if (ret < 0) 
    {
         LOG_ERR("Invalid BT address (err %d)", ret);
         return ret;
    }   

    ret = bt_id_create(&addr, NULL);
    if (ret < 0)
    {
        LOG_ERR("Creating new ID failed (err %d)", ret);
    }

    // Initialize the Bluetooth Subsystem
    ret = bt_enable(NULL);
    if (ret) 
    {
        LOG_ERR("Bluetooth init failed (err %d)", ret);
        return ret;
    }

    LOG_INF("Bluetooth initialized");

    // Start advertising
    advertising_start();

    return 0;
}

int ble_manager_notify_button_pressed(bool button_pressed)
{
    int ret = bt_lbs_send_button_state(button_pressed);
    if (ret < 0)
    {
        LOG_ERR("Failed to send button state notification (err %d)", ret);
        // return ret;
    }

    ret = k_work_submit(&update_adv_data_work);
    if (ret < 0)
    {
        LOG_ERR("Failed to submit work for updating advertising data (err %d)", ret);
    }

    return ret;
}

//------------------------------------------------------------------------------
