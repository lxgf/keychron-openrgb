/*---------------------------------------------------------*\
| ViaEffectEngine.cpp                                       |
|                                                           |
|   Runs one custom effect at a time on its own thread      |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <chrono>
#include "ViaEffectEngine.h"

static unsigned long long via_now_ms()
{
    return((unsigned long long)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
}

ViaEffectEngine::ViaEffectEngine(ViaLightingSync* sync_ptr, ViaSettings* settings_ptr)
{
    sync                = sync_ptr;
    settings            = settings_ptr;
    context.entry       = NULL;
    context.max_effect  = 0;
    running             = false;
    settings_changed    = false;
    active_effect       = 0;

    listener_id = settings->AddListener([this]
    {
        {
            std::lock_guard<std::mutex> guard(engine_lock);
            settings_changed = true;
        }

        engine_cv.notify_one();
    });
}

ViaEffectEngine::~ViaEffectEngine()
{
    Stop();
    settings->RemoveListener(listener_id);
}

void ViaEffectEngine::SetContext(const via_effect_context& new_context)
{
    context = new_context;
}

/*---------------------------------------------------------*\
| Start an effect; selecting the running one again is a     |
| no-op                                                     |
\*---------------------------------------------------------*/
void ViaEffectEngine::Run(unsigned int effect_idx)
{
    if(effect_idx >= via_custom_effect_count)
    {
        return;
    }

    {
        std::lock_guard<std::mutex> guard(engine_lock);

        if(running && active_effect == effect_idx)
        {
            return;
        }
    }

    Stop();

    std::lock_guard<std::mutex> guard(engine_lock);

    running             = true;
    settings_changed    = false;
    active_effect       = effect_idx;
    engine_thread       = std::thread(&ViaEffectEngine::EngineThread, this, effect_idx);
}

void ViaEffectEngine::Stop()
{
    {
        std::lock_guard<std::mutex> guard(engine_lock);

        if(!running)
        {
            return;
        }

        running = false;
    }

    engine_cv.notify_one();

    if(engine_thread.joinable())
    {
        engine_thread.join();
    }
}

void ViaEffectEngine::EngineThread(unsigned int effect_idx)
{
    const via_custom_effect_desc*   desc    = &via_custom_effects[effect_idx];
    ViaCustomEffect*                effect  = desc->create(context);
    via_rgb_state                   state   = sync->GetState();

    effect->Configure(settings->GetSection(desc->settings_section));
    effect->Start(state);

    while(true)
    {
        bool reconfigure;

        {
            std::lock_guard<std::mutex> guard(engine_lock);

            if(!running)
            {
                break;
            }

            reconfigure         = settings_changed;
            settings_changed    = false;
        }

        if(reconfigure)
        {
            effect->Configure(settings->GetSection(desc->settings_section));
        }

        unsigned int wait_ms = effect->Tick(via_now_ms(), &state);

        sync->Apply(state, false);

        std::unique_lock<std::mutex> lock(engine_lock);

        engine_cv.wait_for(lock, std::chrono::milliseconds(wait_ms), [this]{ return(!running || settings_changed); });
    }

    effect->Stop();
    delete effect;
}
