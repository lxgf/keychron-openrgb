/*---------------------------------------------------------*\
| ViaRGBMatrixPlugin.cpp                                    |
|                                                           |
|   OpenRGB plugin exposing stock QMK/VIA RGB Matrix        |
|   keyboards as OpenRGB devices                            |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "LogManager.h"
#include "ViaRGBMatrixPlugin.h"

#ifndef VIA_PLUGIN_VERSION
#define VIA_PLUGIN_VERSION              "0.0.0"
#endif

#ifndef VIA_PLUGIN_COMMIT
#define VIA_PLUGIN_COMMIT               ""
#endif

/*---------------------------------------------------------*\
| OPENRGB_SDK_PROTOCOL_VERSION in OpenRGB 1.1               |
\*---------------------------------------------------------*/
#define VIA_PLUGIN_SDK_PROTOCOL_VERSION 6

/*---------------------------------------------------------*\
| Any value outside the location enum hides the plugin tab |
\*---------------------------------------------------------*/
#define VIA_PLUGIN_LOCATION_NONE        0xFFFFFFFF

ViaRGBMatrixPlugin::ViaRGBMatrixPlugin()
{
    detector = NULL;
}

ViaRGBMatrixPlugin::~ViaRGBMatrixPlugin()
{
    Unload();
}

OpenRGBPluginInfo ViaRGBMatrixPlugin::GetPluginInfo()
{
    OpenRGBPluginInfo info;

    info.Name               = "VIA RGB Matrix";
    info.Description        = "Stock QMK/VIA keyboards with RGB Matrix: hardware effects and single-color Direct mode";
    info.Version            = VIA_PLUGIN_VERSION;
    info.Commit             = VIA_PLUGIN_COMMIT;
    info.URL                = "";
    info.Location           = VIA_PLUGIN_LOCATION_NONE;
    info.Label              = "";
    info.ProtocolVersion    = VIA_PLUGIN_SDK_PROTOCOL_VERSION;

    return(info);
}

unsigned int ViaRGBMatrixPlugin::GetPluginAPIVersion()
{
    return(OPENRGB_PLUGIN_API_VERSION);
}

void ViaRGBMatrixPlugin::Load(OpenRGBPluginAPIInterface* plugin_api_ptr)
{
    plugin_api_ptr->LogEntry(__FILE__, __LINE__, LL_INFO, "[VIA RGB Matrix] Plugin %s loaded", VIA_PLUGIN_VERSION);

    detector = new ViaRGBMatrixDetector(plugin_api_ptr);
    detector->Start();
}

QWidget* ViaRGBMatrixPlugin::GetWidget()
{
    return(NULL);
}

QMenu* ViaRGBMatrixPlugin::GetTrayMenu()
{
    return(NULL);
}

void ViaRGBMatrixPlugin::Unload()
{
    if(detector != NULL)
    {
        delete detector;
        detector = NULL;
    }
}

void ViaRGBMatrixPlugin::OnProfileAboutToLoad()
{
}

void ViaRGBMatrixPlugin::OnProfileLoad(nlohmann::json /*profile_data*/)
{
}

nlohmann::json ViaRGBMatrixPlugin::OnProfileSave()
{
    return(nlohmann::json());
}

unsigned char* ViaRGBMatrixPlugin::OnSDKCommand(unsigned int /*pkt_id*/, unsigned char* /*pkt_data*/, unsigned int* pkt_size)
{
    *pkt_size = 0;
    return(NULL);
}

void ViaRGBMatrixPlugin::ProfileManagerUpdated(unsigned int /*update_reason*/)
{
}

void ViaRGBMatrixPlugin::ResourceManagerUpdated(unsigned int /*update_reason*/)
{
}

void ViaRGBMatrixPlugin::SettingsManagerUpdated(unsigned int /*update_reason*/)
{
}
