/*---------------------------------------------------------*\
| ViaLog.cpp                                                |
|                                                           |
|   Logging through OpenRGB's LogManager via the plugin API |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <atomic>
#include "ViaLog.h"

static std::atomic<OpenRGBPluginAPIInterface*> log_api(NULL);

void via_log_set_api(OpenRGBPluginAPIInterface* api)
{
    log_api = api;
}

OpenRGBPluginAPIInterface* via_log_get_api()
{
    return(log_api);
}
