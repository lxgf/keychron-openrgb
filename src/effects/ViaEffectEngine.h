/*---------------------------------------------------------*\
| ViaEffectEngine.h                                         |
|                                                           |
|   Runs one custom effect at a time on its own thread.     |
|   The thread exists only while a custom mode is active    |
|   and sleeps for exactly as long as the effect asks, so   |
|   a slow effect costs a handful of wakeups per minute.    |
|   Settings changes wake it at once.                       |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <condition_variable>
#include <mutex>
#include <thread>
#include "ViaCustomEffects.h"
#include "ViaLightingSync.h"
#include "ViaSettings.h"

class ViaEffectEngine
{
public:
    ViaEffectEngine(ViaLightingSync* sync_ptr, ViaSettings* settings_ptr);
    ~ViaEffectEngine();

    void    SetContext(const via_effect_context& new_context);

    void    Run(unsigned int effect_idx);
    void    Stop();

private:
    ViaLightingSync*            sync;
    ViaSettings*                settings;
    via_effect_context          context;
    unsigned int                listener_id;

    std::thread                 engine_thread;
    std::mutex                  engine_lock;
    std::condition_variable     engine_cv;
    bool                        running;
    bool                        settings_changed;
    unsigned int                active_effect;

    void    EngineThread(unsigned int effect_idx);
};
