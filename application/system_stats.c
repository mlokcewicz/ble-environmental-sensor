//------------------------------------------------------------------------------

/// @file system_stats.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include "system_stats.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/sys_heap.h>
#include <zephyr/debug/cpu_load.h>

//------------------------------------------------------------------------------

LOG_MODULE_REGISTER(system_stats, CONFIG_LOG_DEFAULT_LEVEL);

extern struct sys_heap _system_heap;

//------------------------------------------------------------------------------

static void thread_system_stats_cb(const struct k_thread *thread, void *user_data)
{
    ARG_UNUSED(user_data);

    size_t free_stack = 0U;
    size_t total_stack = 0U;
    size_t used_stack = 0U;
    uint32_t used_percent = 0U;
    int ret;

    const char *name = k_thread_name_get((k_tid_t)thread);
    if (name == NULL)
    {
        name = "unnamed";
    }

#if defined(CONFIG_THREAD_STACK_INFO)
    total_stack = thread->stack_info.size;
#endif

    ret = k_thread_stack_space_get((k_tid_t)thread, &free_stack);
    if (ret != 0)
    {
        LOG_ERR("Thread %-16s | stack stats failed (%d)", name, ret);
        return;
    }

    if (total_stack >= free_stack)
    {
        used_stack = total_stack - free_stack;
    }
    else
    {
        used_stack = 0U;
    }

    if (total_stack > 0U)
    {
        used_percent = (uint32_t)((used_stack * 100U) / total_stack);
    }

    LOG_INF("Thread %-16s | Free: %u B | Used: %u B | Total: %u B | Used: %u%%",
            name,
            (uint32_t)free_stack,
            (uint32_t)used_stack,
            (uint32_t)total_stack,
            used_percent);
}

//------------------------------------------------------------------------------

int system_stats_print(void)
{
    struct sys_memory_stats heap_stats;
    uint32_t cpu_load = 0U;
    int ret;

    LOG_INF("===== THREAD STATS =====");
    k_thread_foreach(thread_system_stats_cb, NULL);

    LOG_INF("===== HEAP STATS =====");

    ret = sys_heap_runtime_stats_get(&_system_heap, &heap_stats);
    if (ret != 0)
    {
        LOG_ERR("Heap stats failed (%d)", ret);
    }
    else
    {
        uint32_t total_heap = heap_stats.free_bytes + heap_stats.allocated_bytes;
        uint32_t used_heap_percent = 0U;

        if (total_heap > 0U)
        {
            used_heap_percent = (heap_stats.allocated_bytes * 100U) / total_heap;
        }

        LOG_INF("Heap | Free: %u B | Allocated: %u B | Total: %u B | Used: %u%% | Max Used: %u B",
                (uint32_t)heap_stats.free_bytes,
                (uint32_t)heap_stats.allocated_bytes,
                total_heap,
                used_heap_percent,
                (uint32_t)heap_stats.max_allocated_bytes);
    }

#if defined(CONFIG_CPU_LOAD)
    cpu_load_get(&cpu_load);
    LOG_INF("CPU load: %u%%", cpu_load);
#endif

    LOG_INF("Uptime: %lld ms", k_uptime_get());

    return 0;
}

//------------------------------------------------------------------------------
