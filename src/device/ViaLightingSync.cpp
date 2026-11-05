/*---------------------------------------------------------*\
| ViaLightingSync.cpp                                       |
|                                                           |
|   Diffing state sync to the keyboard. One VIA round trip  |
|   costs ~2 ms on full-speed USB, so an unchanged value is |
|   never resent.                                           |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <cstring>
#include "ViaLightingSync.h"
#include "ViaLog.h"

ViaLightingSync::ViaLightingSync(ViaRGBMatrixController* controller_ptr)
{
    controller  = controller_ptr;
    sent_valid  = false;

    memset(&sent, 0, sizeof(sent));
}

void ViaLightingSync::Init(const via_rgb_state& current)
{
    sent        = current;
    sent_valid  = true;
}

const via_rgb_state& ViaLightingSync::GetState()
{
    return(sent);
}

void ViaLightingSync::Apply(const via_rgb_state& next, bool save)
{
    bool ok = true;

    if(ok && (!sent_valid || next.effect != sent.effect))
    {
        ok = controller->SetEffect(next.effect);
    }

    if(ok && (!sent_valid || next.hue != sent.hue || next.sat != sent.sat))
    {
        ok = controller->SetColor(next.hue, next.sat);
    }

    if(ok && (!sent_valid || next.brightness != sent.brightness))
    {
        ok = controller->SetBrightness(next.brightness);
    }

    if(ok && (!sent_valid || next.speed != sent.speed))
    {
        ok = controller->SetSpeed(next.speed);
    }

    if(ok && save)
    {
        ok = controller->Save();
    }

    /*-----------------------------------------------------*\
    | After a failure the device state is unknown: resend   |
    | everything on the next call                           |
    \*-----------------------------------------------------*/
    sent        = next;
    sent_valid  = ok;

    if(!ok)
    {
        VIA_LOG(LL_WARNING, "HID transfer failed on %s", controller->GetPath().c_str());
    }
}
