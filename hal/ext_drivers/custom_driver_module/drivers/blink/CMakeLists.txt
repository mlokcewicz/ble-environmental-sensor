# Copyright (c) 2024 Nordic Semiconductor ASA
# SPDX-License-Identifier: Apache-2.0

zephyr_library_named(blink_gpio_led) # instead of zephyr_library() to shorten library name and workaround CMake's object path length limit
zephyr_library_sources_ifdef(CONFIG_BLINK_GPIO_LED gpio_led.c)
