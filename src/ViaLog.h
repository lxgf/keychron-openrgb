/*---------------------------------------------------------*\
| ViaLog.h                                                  |
|                                                           |
|   Logging through OpenRGB's LogManager via the plugin API |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "LogManager.h"
#include "OpenRGBPluginInterface.h"

void                        via_log_set_api(OpenRGBPluginAPIInterface* api);
OpenRGBPluginAPIInterface*  via_log_get_api();

/*---------------------------------------------------------*\
| First variadic argument (format) must be a string literal |
\*---------------------------------------------------------*/
#define VIA_LOG(level, ...)                                                                         \
    do                                                                                              \
    {                                                                                               \
        OpenRGBPluginAPIInterface* via_log_api_ptr = via_log_get_api();                             \
        if(via_log_api_ptr != NULL)                                                                 \
        {                                                                                           \
            via_log_api_ptr->LogEntry(__FILE__, __LINE__, level, "[VIA RGB Matrix] " __VA_ARGS__);  \
        }                                                                                           \
    } while(0)
