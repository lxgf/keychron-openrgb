/*---------------------------------------------------------*\
| ViaSystemSensors.h                                        |
|                                                           |
|   CPU, RAM and GPU load in percent. CPU and RAM come from |
|   plain Win32 calls; GPU uses the PDH "GPU Engine" 3D     |
|   counters, opened only when first asked for.             |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

enum
{
    VIA_SENSOR_CPU,
    VIA_SENSOR_RAM,
    VIA_SENSOR_GPU,
};

class ViaSystemSensors
{
public:
    ViaSystemSensors();
    ~ViaSystemSensors();

    float   Read(int sensor);

private:
    unsigned long long  last_idle;
    unsigned long long  last_total;
    void*               gpu_query;
    void*               gpu_counter;
    bool                gpu_failed;

    float   ReadCPU();
    float   ReadRAM();
    float   ReadGPU();
};
