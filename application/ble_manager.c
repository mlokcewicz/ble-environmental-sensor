//------------------------------------------------------------------------------

/// @file ble_manager.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include "ble_manager.h"

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gap.h>
#include <zephyr/bluetooth/gatt.h>

#include <zephyr/bluetooth/conn.h>

#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/addr.h>

#include <services/my_lbs.h>

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
static struct bt_gatt_exchange_params exchange_params;

static struct k_work update_adv_data_work;
static struct k_work adv_work;

static ble_manager_connection_state_cb connection_state_callback = NULL;

//------------------------------------------------------------------------------

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

//------------------------------------------------------------------------------

static void advertising_start(void)
{
    k_work_submit(&adv_work);
}

static void update_phy(struct bt_conn *conn)
{
    const struct bt_conn_le_phy_param preferred_phy = 
    {
        .options = BT_CONN_LE_PHY_OPT_NONE,
        .pref_rx_phy = BT_GAP_LE_PHY_2M,
        .pref_tx_phy = BT_GAP_LE_PHY_2M,
    };
    
    int ret = bt_conn_le_phy_update(conn, &preferred_phy);
    if (ret)
    {
        LOG_ERR("bt_conn_le_phy_update() returned %d", ret);
    }
}

static void update_data_length(struct bt_conn *conn)
{
    struct bt_conn_le_data_len_param my_data_len = 
    {
        .tx_max_len = BT_GAP_DATA_LEN_MAX,
        .tx_max_time = BT_GAP_DATA_TIME_MAX,
    };
    int ret = bt_conn_le_data_len_update(my_conn, &my_data_len);
    if (ret) 
    {
        LOG_ERR("data_len_update failed (err %d)", ret);
    }
}

static void exchange_func(struct bt_conn *conn, uint8_t att_err, struct bt_gatt_exchange_params *params);

static void update_mtu(struct bt_conn *conn)
{
    exchange_params.func = exchange_func;
    
    int ret = bt_gatt_exchange_mtu(conn, &exchange_params);
    if (ret) 
    {
        LOG_ERR("bt_gatt_exchange_mtu failed (err %d)", ret);
    }
}

//------------------------------------------------------------------------------

static void exchange_func(struct bt_conn *conn, uint8_t att_err, struct bt_gatt_exchange_params *params)
{
	LOG_INF("MTU exchange %s", att_err == 0 ? "successful" : "failed");
    if (!att_err) 
    {
        uint16_t payload_mtu = bt_gatt_get_mtu(conn) - 3;   // 3 bytes used for Attribute headers.
        LOG_INF("New MTU: %d bytes", payload_mtu);
    }
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

    struct bt_conn_info info;
	int ret = bt_conn_get_info(conn, &info);
	if (ret) 
    {
		LOG_ERR("bt_conn_get_info() returned %d", ret);
		return;
	}

    double connection_interval = BT_GAP_US_TO_CONN_INTERVAL(info.le.interval_us) * 1.25; // in ms
    uint16_t supervision_timeout = info.le.timeout * 10; // in ms
    
    LOG_INF("Connection parameters: interval %.2f ms, latency %d intervals, timeout %d ms", connection_interval, info.le.latency, supervision_timeout);

    update_phy(my_conn);

    k_sleep(K_MSEC(1000));  // Delay added to avoid link layer collisions.
    
    update_data_length(my_conn);
    update_mtu(my_conn);

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

static void on_security_changed(struct bt_conn *conn, bt_security_t level, enum bt_security_err err)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	if (!err) 
    {
		LOG_INF("Security changed: %s level %u\n", addr, level);
	} 
    else 
    {
		LOG_INF("Security failed: %s level %u err %d\n", addr, level, err);
	}
}

static void on_le_param_updated(struct bt_conn *conn, uint16_t interval, uint16_t latency, uint16_t timeout)
{
    double connection_interval = interval * 1.25; // in ms
    uint16_t supervision_timeout = timeout * 10; // in ms

    LOG_INF("Connection parameters updated: interval %.2f ms, latency %d intervals, timeout %d ms", connection_interval, latency, supervision_timeout);
}

static void on_le_phy_updated(struct bt_conn *conn, struct bt_conn_le_phy_info *param)
{
    // PHY Updated
    if (param->tx_phy == BT_CONN_LE_TX_POWER_PHY_1M)
    {
        LOG_INF("PHY updated. New PHY: 1M");
    }
    else if (param->tx_phy == BT_CONN_LE_TX_POWER_PHY_2M)
    {
        LOG_INF("PHY updated. New PHY: 2M");
    }
    else if (param->tx_phy == BT_CONN_LE_TX_POWER_PHY_CODED_S8)
    {
        LOG_INF("PHY updated. New PHY: Long Range");
    }
}

void on_le_data_len_updated(struct bt_conn *conn, struct bt_conn_le_data_len_info *info)
{
    uint16_t tx_len     = info->tx_max_len; 
    uint16_t tx_time    = info->tx_max_time;
    uint16_t rx_len     = info->rx_max_len;
    uint16_t rx_time    = info->rx_max_time;
    LOG_INF("Data length updated. Length %d/%d bytes, time %d/%d us", tx_len, rx_len, tx_time, rx_time);
}

BT_CONN_CB_DEFINE(conn_callbacks) = 
{
	.connected = on_connected,
    .disconnected = on_disconnected,
	.recycled = on_recycled,
    .security_changed = on_security_changed,
    .le_param_updated = on_le_param_updated,
    .le_phy_updated = on_le_phy_updated,
    .le_data_len_updated = on_le_data_len_updated,
};

static void auth_passkey_display(struct bt_conn *conn, unsigned int passkey)
{
	char addr[BT_ADDR_LE_STR_LEN];
	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
	LOG_INF("Passkey for %s: %06u\n", addr, passkey);
}

static void auth_cancel(struct bt_conn *conn)
{
	char addr[BT_ADDR_LE_STR_LEN];
	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
	LOG_INF("Pairing cancelled: %s\n", addr);
}

static struct bt_conn_auth_cb conn_auth_callbacks = 
{
	.passkey_display = auth_passkey_display,
	.cancel = auth_cancel,
};

//------------------------------------------------------------------------------

int ble_manager_init(struct ble_manager_cfg *cfg)
{
    if (!cfg)
    {
        LOG_ERR("Invalid configuration pointer");
        return -1;
    }

    k_work_init(&adv_work, adv_work_handler);
    k_work_init(&update_adv_data_work, update_adv_data_work_handler);

    connection_state_callback = cfg->connection_state_cb;

    int ret = bt_conn_auth_cb_register(&conn_auth_callbacks);
    if (ret)
    {
         LOG_ERR("Failed to register authentication callbacks (err %d)", ret);
         return ret;
    }

    struct my_lbs_cb lbs_callbacks = 
    {
        .led_set_cb = cfg->led_set_cb,
        .button_get_cb = cfg->button_get_cb,
    };

    ret = my_lbs_init(&lbs_callbacks);
    if (ret < 0)
    {
        LOG_ERR("Failed to initialize My LBS Service (err %d)", ret);
        return ret;
    }

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
