//------------------------------------------------------------------------------

/// @file ui_manager.h
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#ifndef UI_MANAGER_H_
#define UI_MANAGER_H_

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#include <stdbool.h>

//------------------------------------------------------------------------------

/// @brief Initializes the UI Manager module.
/// @return 0 on success, or a negative error code on failure.
int ui_manager_init(void);

/// @brief Processes UI Manager tasks
/// @note This function should be called periodically
/// @return 0 on success, or a negative error code on failure.
int ui_manager_process(void);

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif /* UI_MANAGER_H_ */

//------------------------------------------------------------------------------
