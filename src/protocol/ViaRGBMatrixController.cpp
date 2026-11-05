/*---------------------------------------------------------*\
| ViaRGBMatrixController.cpp                                |
|                                                           |
|   HID layer for stock QMK/VIA keyboards, RGB Matrix       |
|   channel (custom_set_value / custom_get_value / save)    |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <cstring>
#include "ViaRGBMatrixController.h"

#define VIA_SERIAL_MAX_LENGTH           128

ViaRGBMatrixController::ViaRGBMatrixController(hid_device* dev_handle, const char* dev_path)
{
    dev                 = dev_handle;
    path                = dev_path;
    protocol_version    = 0;
    needs_resync        = false;
}

ViaRGBMatrixController::~ViaRGBMatrixController()
{
    hid_close(dev);
}

const std::string& ViaRGBMatrixController::GetPath()
{
    return(path);
}

std::string ViaRGBMatrixController::GetLocation()
{
    return("HID: " + path);
}

std::string ViaRGBMatrixController::GetSerial()
{
    wchar_t     serial_string[VIA_SERIAL_MAX_LENGTH];
    std::string serial;

    if(hid_get_serial_number_string(dev, serial_string, VIA_SERIAL_MAX_LENGTH) == 0)
    {
        for(unsigned int char_idx = 0; char_idx < VIA_SERIAL_MAX_LENGTH && serial_string[char_idx] != 0; char_idx++)
        {
            serial.push_back((char)serial_string[char_idx]);
        }
    }

    return(serial);
}

unsigned short ViaRGBMatrixController::GetProtocolVersion()
{
    return(protocol_version);
}

/*---------------------------------------------------------*\
| Send one 32-byte packet and wait for the matching echo.   |
| VIA answers every command with the request bytes plus     |
| the result; an unhandled command comes back with 0xFF in  |
| byte 0. The response overwrites data on success.          |
\*---------------------------------------------------------*/
bool ViaRGBMatrixController::Transfer(unsigned char* data)
{
    unsigned char usb_buf[VIA_PACKET_SIZE + 1];

    std::lock_guard<std::mutex> guard(transfer_lock);

    /*-----------------------------------------------------*\
    | A late answer to a timed-out request may still be     |
    | queued; drain only after a failure                    |
    \*-----------------------------------------------------*/
    if(needs_resync)
    {
        for(unsigned int stale_idx = 0; stale_idx < VIA_MAX_STALE_REPORTS; stale_idx++)
        {
            if(hid_read_timeout(dev, usb_buf, VIA_PACKET_SIZE, 0) <= 0)
            {
                break;
            }
        }

        needs_resync = false;
    }

    usb_buf[0] = 0x00;
    memcpy(&usb_buf[1], data, VIA_PACKET_SIZE);

    if(hid_write(dev, usb_buf, VIA_PACKET_SIZE + 1) < 0)
    {
        needs_resync = true;
        return(false);
    }

    for(unsigned int attempt_idx = 0; attempt_idx < VIA_MAX_STALE_REPORTS; attempt_idx++)
    {
        if(hid_read_timeout(dev, usb_buf, VIA_PACKET_SIZE, VIA_READ_TIMEOUT_MS) <= 0)
        {
            break;
        }

        if(usb_buf[0] == VIA_CMD_UNHANDLED)
        {
            return(false);
        }

        if(usb_buf[0] == data[0] && (data[0] == VIA_CMD_GET_PROTOCOL_VERSION || usb_buf[1] == data[1]))
        {
            memcpy(data, usb_buf, VIA_PACKET_SIZE);
            return(true);
        }
    }

    needs_resync = true;
    return(false);
}

bool ViaRGBMatrixController::Handshake()
{
    unsigned char buf[VIA_PACKET_SIZE];

    memset(buf, 0, VIA_PACKET_SIZE);
    buf[0] = VIA_CMD_GET_PROTOCOL_VERSION;

    if(!Transfer(buf))
    {
        return(false);
    }

    protocol_version = (buf[1] << 8) | buf[2];

    return(protocol_version != 0);
}

bool ViaRGBMatrixController::GetValue(unsigned char value_id, unsigned char* out, unsigned int count)
{
    unsigned char buf[VIA_PACKET_SIZE];

    memset(buf, 0, VIA_PACKET_SIZE);
    buf[0] = VIA_CMD_CUSTOM_GET_VALUE;
    buf[1] = VIA_CHANNEL_RGB_MATRIX;
    buf[2] = value_id;

    if(!Transfer(buf) || buf[2] != value_id)
    {
        return(false);
    }

    memcpy(out, &buf[3], count);

    return(true);
}

bool ViaRGBMatrixController::SetValue(unsigned char value_id, const unsigned char* in, unsigned int count)
{
    unsigned char buf[VIA_PACKET_SIZE];

    memset(buf, 0, VIA_PACKET_SIZE);
    buf[0] = VIA_CMD_CUSTOM_SET_VALUE;
    buf[1] = VIA_CHANNEL_RGB_MATRIX;
    buf[2] = value_id;
    memcpy(&buf[3], in, count);

    return(Transfer(buf));
}

bool ViaRGBMatrixController::ReadState(via_rgb_state* state)
{
    unsigned char hue_sat[2];

    if(!GetValue(VIA_RGB_MATRIX_EFFECT,       &state->effect,     1)
    || !GetValue(VIA_RGB_MATRIX_BRIGHTNESS,   &state->brightness, 1)
    || !GetValue(VIA_RGB_MATRIX_EFFECT_SPEED, &state->speed,      1)
    || !GetValue(VIA_RGB_MATRIX_COLOR,        hue_sat,            2))
    {
        return(false);
    }

    state->hue = hue_sat[0];
    state->sat = hue_sat[1];

    return(true);
}

/*---------------------------------------------------------*\
| QMK clamps an out-of-range mode to RGB_MATRIX_EFFECT_MAX-1|
| so setting 0xFF and reading it back yields the last       |
| enabled effect id. The original effect is restored and    |
| nothing is written to EEPROM.                             |
\*---------------------------------------------------------*/
bool ViaRGBMatrixController::ProbeMaxEffect(unsigned char* max_effect)
{
    unsigned char original_effect;

    if(!GetValue(VIA_RGB_MATRIX_EFFECT, &original_effect, 1))
    {
        return(false);
    }

    bool ok = SetEffect(0xFF) && GetValue(VIA_RGB_MATRIX_EFFECT, max_effect, 1);

    SetEffect(original_effect);

    return(ok && *max_effect != 0xFF && *max_effect != 0);
}

bool ViaRGBMatrixController::SetBrightness(unsigned char brightness)
{
    return(SetValue(VIA_RGB_MATRIX_BRIGHTNESS, &brightness, 1));
}

bool ViaRGBMatrixController::SetEffect(unsigned char effect)
{
    return(SetValue(VIA_RGB_MATRIX_EFFECT, &effect, 1));
}

bool ViaRGBMatrixController::SetSpeed(unsigned char speed)
{
    return(SetValue(VIA_RGB_MATRIX_EFFECT_SPEED, &speed, 1));
}

bool ViaRGBMatrixController::SetColor(unsigned char hue, unsigned char sat)
{
    unsigned char hue_sat[2] = { hue, sat };

    return(SetValue(VIA_RGB_MATRIX_COLOR, hue_sat, 2));
}

bool ViaRGBMatrixController::Save()
{
    unsigned char buf[VIA_PACKET_SIZE];

    memset(buf, 0, VIA_PACKET_SIZE);
    buf[0] = VIA_CMD_CUSTOM_SAVE;
    buf[1] = VIA_CHANNEL_RGB_MATRIX;

    return(Transfer(buf));
}
