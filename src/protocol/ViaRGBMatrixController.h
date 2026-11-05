/*---------------------------------------------------------*\
| ViaRGBMatrixController.h                                  |
|                                                           |
|   HID layer for stock QMK/VIA keyboards, RGB Matrix       |
|   channel (custom_set_value / custom_get_value / save)    |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <mutex>
#include <string>
#include <hidapi.h>

#define VIA_RAW_HID_USAGE_PAGE          0xFF60
#define VIA_RAW_HID_USAGE               0x61
#define VIA_PACKET_SIZE                 32
#define VIA_READ_TIMEOUT_MS             100
#define VIA_MAX_STALE_REPORTS           4

enum
{
    VIA_CMD_GET_PROTOCOL_VERSION        = 0x01,
    VIA_CMD_CUSTOM_SET_VALUE            = 0x07,
    VIA_CMD_CUSTOM_GET_VALUE            = 0x08,
    VIA_CMD_CUSTOM_SAVE                 = 0x09,
    VIA_CMD_UNHANDLED                   = 0xFF,
};

enum
{
    VIA_CHANNEL_RGB_MATRIX              = 0x03,
};

enum
{
    VIA_RGB_MATRIX_BRIGHTNESS           = 0x01,
    VIA_RGB_MATRIX_EFFECT               = 0x02,
    VIA_RGB_MATRIX_EFFECT_SPEED         = 0x03,
    VIA_RGB_MATRIX_COLOR                = 0x04,
};

typedef struct
{
    unsigned char   effect;
    unsigned char   brightness;
    unsigned char   speed;
    unsigned char   hue;
    unsigned char   sat;
} via_rgb_state;

class ViaRGBMatrixController
{
public:
    ViaRGBMatrixController(hid_device* dev_handle, const char* dev_path);
    ~ViaRGBMatrixController();

    const std::string&  GetPath();
    std::string         GetLocation();
    std::string         GetSerial();
    unsigned short      GetProtocolVersion();

    bool                Handshake();
    bool                ReadState(via_rgb_state* state);
    bool                ProbeMaxEffect(unsigned char* max_effect);

    bool                SetBrightness(unsigned char brightness);
    bool                SetEffect(unsigned char effect);
    bool                SetSpeed(unsigned char speed);
    bool                SetColor(unsigned char hue, unsigned char sat);
    bool                Save();

private:
    hid_device*         dev;
    std::string         path;
    unsigned short      protocol_version;
    bool                needs_resync;
    std::mutex          transfer_lock;

    bool                Transfer(unsigned char* data);
    bool                GetValue(unsigned char value_id, unsigned char* out, unsigned int count);
    bool                SetValue(unsigned char value_id, const unsigned char* in, unsigned int count);
};
