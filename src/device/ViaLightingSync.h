/*---------------------------------------------------------*\
| ViaLightingSync.h                                         |
|                                                           |
|   Keeps the keyboard in step with the requested lighting  |
|   state, sending only the values that changed.            |
|                                                           |
|   Not thread-safe by design: OpenRGB serializes every     |
|   Device* callback of a controller under its DeviceMutex  |
|   and already coalesces frames on its device thread, so   |
|   Apply() runs synchronously there with no extra locking. |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "ViaRGBMatrixController.h"

class ViaLightingSync
{
public:
    ViaLightingSync(ViaRGBMatrixController* controller_ptr);

    void                    Init(const via_rgb_state& current);
    const via_rgb_state&    GetState();
    void                    Apply(const via_rgb_state& next, bool save);

private:
    ViaRGBMatrixController* controller;
    via_rgb_state           sent;
    bool                    sent_valid;
};
