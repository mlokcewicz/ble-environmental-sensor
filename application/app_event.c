//------------------------------------------------------------------------------

/// @file app_event.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include "app_event.h"

//------------------------------------------------------------------------------

ZBUS_CHAN_DEFINE
(
    ble_control_chan,                       // Channel name
    struct app_event,                       // Message type
    NULL,                                   // Validator function
    NULL,                                   // User data
    ZBUS_OBSERVERS_EMPTY,                   // No observers - will be added in managers
    ZBUS_MSG_INIT(.type = APP_EVENT_MAX)    // Initial message value (type = APP_EVENT_MAX means no event)
);

ZBUS_CHAN_DEFINE
(
    env_control_chan,                       // Channel name
    struct app_event,                       // Message type
    NULL,                                   // Validator function
    NULL,                                   // User data
    ZBUS_OBSERVERS_EMPTY,                   // No observers - will be added in managers
    ZBUS_MSG_INIT(.type = APP_EVENT_MAX)    // Initial message value (type = APP_EVENT_MAX means no event)
);

ZBUS_CHAN_DEFINE
(
    ui_control_chan,                        // Channel name
    struct app_event,                       // Message type
    NULL,                                   // Validator function
    NULL,                                   // User data
    ZBUS_OBSERVERS_EMPTY,                   // No observers - will be added in managers
    ZBUS_MSG_INIT(.type = APP_EVENT_MAX)    // Initial message value (type = APP_EVENT_MAX means no event)
);

K_MSGQ_DEFINE(sensor_event_queue, sizeof(struct app_event), APP_EVENT_SENSOR_DATA_QUEUE_SIZE, 1);

//------------------------------------------------------------------------------
