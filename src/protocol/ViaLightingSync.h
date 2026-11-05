/*---------------------------------------------------------*\
| ViaLightingSync.h                                         |
|                                                           |
|   Keeps the keyboard in step with the requested lighting  |
|   state, sending only the values that changed.            |
|                                                           |
|   Called from OpenRGB's per-device call thread (already   |
|   coalesced by the host) and from the custom effect       |
|   engine thread, never both at once in practice; the lock |
|   only guards the hand-over between them.                 |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <mutex>
#include "ViaRGBMatrixController.h"

class ViaLightingSync
{
public:
    ViaLightingSync(ViaRGBMatrixController* controller_ptr);

    void            Init(const via_rgb_state& current);
    via_rgb_state   GetState();
    void            Apply(const via_rgb_state& next, bool save);

private:
    ViaRGBMatrixController* controller;
    std::mutex              sync_lock;
    via_rgb_state           sent;
    bool                    sent_valid;
};
