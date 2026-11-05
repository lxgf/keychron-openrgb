/*---------------------------------------------------------*\
| ViaLightingSync.cpp                                       |
|                                                           |
|   Coalescing, diffing state sync to the keyboard          |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <cstring>
#include "ViaLightingSync.h"
#include "ViaLog.h"

static bool via_state_equal(const via_rgb_state& a, const via_rgb_state& b)
{
    return(a.effect     == b.effect
        && a.brightness == b.brightness
        && a.speed      == b.speed
        && a.hue        == b.hue
        && a.sat        == b.sat);
}

ViaLightingSync::ViaLightingSync(ViaRGBMatrixController* controller_ptr, ViaWorker* worker_ptr)
{
    controller      = controller_ptr;
    worker          = worker_ptr;
    flush_queued    = false;
    save_queued     = false;
    sent_valid      = false;

    memset(&requested, 0, sizeof(requested));
    memset(&sent,      0, sizeof(sent));
}

void ViaLightingSync::Init(const via_rgb_state& current)
{
    std::lock_guard<std::mutex> guard(request_lock);

    requested   = current;
    sent        = current;
    sent_valid  = true;
}

via_rgb_state ViaLightingSync::GetRequested()
{
    std::lock_guard<std::mutex> guard(request_lock);

    return(requested);
}

/*---------------------------------------------------------*\
| Called from OpenRGB threads (UI, effects, SDK) at up to   |
| frame rate: an unchanged frame returns without waking the |
| worker, and a burst of frames costs a single flush.       |
\*---------------------------------------------------------*/
void ViaLightingSync::Request(const via_rgb_state& next, bool save)
{
    bool post_flush;

    {
        std::lock_guard<std::mutex> guard(request_lock);

        if(!save && via_state_equal(next, requested))
        {
            return;
        }

        requested       = next;
        save_queued     = save_queued || save;
        post_flush      = !flush_queued;
        flush_queued    = true;
    }

    if(post_flush)
    {
        worker->Post([this]{ Flush(); });
    }
}

void ViaLightingSync::Flush()
{
    via_rgb_state   target;
    bool            save;

    {
        std::lock_guard<std::mutex> guard(request_lock);

        target          = requested;
        save            = save_queued;
        flush_queued    = false;
        save_queued     = false;
    }

    bool ok = true;

    if(ok && (!sent_valid || target.effect != sent.effect))
    {
        ok = controller->SetEffect(target.effect);
    }

    if(ok && (!sent_valid || target.hue != sent.hue || target.sat != sent.sat))
    {
        ok = controller->SetColor(target.hue, target.sat);
    }

    if(ok && (!sent_valid || target.brightness != sent.brightness))
    {
        ok = controller->SetBrightness(target.brightness);
    }

    if(ok && (!sent_valid || target.speed != sent.speed))
    {
        ok = controller->SetSpeed(target.speed);
    }

    if(ok && save)
    {
        ok = controller->Save();
    }

    /*-----------------------------------------------------*\
    | After a failure the device state is unknown: resend   |
    | everything on the next flush                          |
    \*-----------------------------------------------------*/
    sent        = target;
    sent_valid  = ok;

    if(!ok)
    {
        VIA_LOG(LL_WARNING, "HID transfer failed on %s", controller->GetPath().c_str());
    }
}
