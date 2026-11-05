/*---------------------------------------------------------*\
| ViaLightingSync.h                                         |
|                                                           |
|   Keeps the keyboard in step with the requested lighting  |
|   state. Requests are coalesced (only the latest state is |
|   sent, at most one flush is queued) and only the values  |
|   that changed go over HID.                               |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <mutex>
#include "ViaRGBMatrixController.h"
#include "ViaWorker.h"

class ViaLightingSync
{
public:
    ViaLightingSync(ViaRGBMatrixController* controller_ptr, ViaWorker* worker_ptr);

    void            Init(const via_rgb_state& current);
    via_rgb_state   GetRequested();
    void            Request(const via_rgb_state& next, bool save);

private:
    ViaRGBMatrixController* controller;
    ViaWorker*              worker;

    /*-----------------------------------------------------*\
    | Guarded by request_lock                               |
    \*-----------------------------------------------------*/
    std::mutex              request_lock;
    via_rgb_state           requested;
    bool                    flush_queued;
    bool                    save_queued;

    /*-----------------------------------------------------*\
    | Worker thread only                                    |
    \*-----------------------------------------------------*/
    via_rgb_state           sent;
    bool                    sent_valid;

    void            Flush();
};
