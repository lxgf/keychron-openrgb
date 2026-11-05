/*---------------------------------------------------------*\
| ViaRGBMatrixPlugin.cpp                                    |
|                                                           |
|   OpenRGB plugin exposing stock QMK/VIA RGB Matrix        |
|   keyboards as OpenRGB devices                            |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "ViaLog.h"
#include "ViaRGBMatrixPlugin.h"
#include "ViaSettingsWidget.h"
#include "ViaTranslation.h"

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

ViaRGBMatrixPlugin::ViaRGBMatrixPlugin()
{
    worker   = NULL;
    detector = NULL;
    settings = NULL;
}

ViaRGBMatrixPlugin::~ViaRGBMatrixPlugin()
{
    Unload();
}

OpenRGBPluginInfo ViaRGBMatrixPlugin::GetPluginInfo()
{
    OpenRGBPluginInfo info;

    info.Name               = "VIA RGB Matrix";
    info.Description        = via_tr("Stock QMK/VIA keyboards with RGB Matrix: firmware effects, single-color Direct mode and host-driven custom effects").toStdString();
    info.Version            = VIA_PLUGIN_VERSION;
    info.Commit             = VIA_PLUGIN_COMMIT;
    info.URL                = "";
    info.Location           = OPENRGB_PLUGIN_LOCATION_SETTINGS;
    info.Label              = "VIA RGB Matrix";
    info.ProtocolVersion    = VIA_PLUGIN_SDK_PROTOCOL_VERSION;

    return(info);
}

unsigned int ViaRGBMatrixPlugin::GetPluginAPIVersion()
{
    return(OPENRGB_PLUGIN_API_VERSION);
}

void ViaRGBMatrixPlugin::Load(OpenRGBPluginAPIInterface* plugin_api_ptr)
{
    via_log_set_api(plugin_api_ptr);
    VIA_LOG(LL_INFO, "plugin %s loaded", VIA_PLUGIN_VERSION);

    settings = new ViaSettings(plugin_api_ptr);
    worker   = new ViaWorker();
    detector = new ViaRGBMatrixDetector(plugin_api_ptr, worker, settings);

    worker->Start();
    detector->Start();
}

QWidget* ViaRGBMatrixPlugin::GetWidget()
{
    if(widget.isNull() && settings != NULL)
    {
        widget = new ViaSettingsWidget(settings);
    }

    return(widget.data());
}

QMenu* ViaRGBMatrixPlugin::GetTrayMenu()
{
    return(NULL);
}

/*---------------------------------------------------------*\
| The DLL is unmapped right after this: every device must  |
| be unregistered and the worker joined before returning.  |
| OpenRGB only removes the tab, so the settings widget,    |
| whose code lives in this DLL, is deleted here unless Qt  |
| already did so with the main window.                     |
\*---------------------------------------------------------*/
void ViaRGBMatrixPlugin::Unload()
{
    if(worker == NULL)
    {
        return;
    }

    delete widget.data();

    detector->Stop();
    worker->Stop();

    delete detector;
    delete worker;
    delete settings;
    detector = NULL;
    worker   = NULL;
    settings = NULL;

    via_log_set_api(NULL);
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
