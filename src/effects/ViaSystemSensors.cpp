/*---------------------------------------------------------*\
| ViaSystemSensors.cpp                                      |
|                                                           |
|   CPU, RAM and GPU load in percent                        |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <vector>
#include <windows.h>
#include <pdh.h>
#include <pdhmsg.h>
#include "ViaLog.h"
#include "ViaSystemSensors.h"

#define GPU_3D_COUNTER_PATH             L"\\GPU Engine(*engtype_3D)\\Utilization Percentage"

static unsigned long long filetime_to_u64(const FILETIME& time)
{
    return(((unsigned long long)time.dwHighDateTime << 32) | time.dwLowDateTime);
}

ViaSystemSensors::ViaSystemSensors()
{
    last_idle   = 0;
    last_total  = 0;
    gpu_query   = NULL;
    gpu_counter = NULL;
    gpu_failed  = false;
}

ViaSystemSensors::~ViaSystemSensors()
{
    if(gpu_query != NULL)
    {
        PdhCloseQuery((PDH_HQUERY)gpu_query);
    }
}

float ViaSystemSensors::Read(int sensor)
{
    switch(sensor)
    {
        case VIA_SENSOR_RAM:    return(ReadRAM());
        case VIA_SENSOR_GPU:    return(ReadGPU());
        default:                return(ReadCPU());
    }
}

/*---------------------------------------------------------*\
| Busy share of all cores since the previous call           |
\*---------------------------------------------------------*/
float ViaSystemSensors::ReadCPU()
{
    FILETIME idle_time;
    FILETIME kernel_time;
    FILETIME user_time;

    if(!GetSystemTimes(&idle_time, &kernel_time, &user_time))
    {
        return(0.0f);
    }

    unsigned long long idle     = filetime_to_u64(idle_time);
    unsigned long long total    = filetime_to_u64(kernel_time) + filetime_to_u64(user_time);
    unsigned long long d_idle   = idle  - last_idle;
    unsigned long long d_total  = total - last_total;
    bool               first    = (last_total == 0);

    last_idle   = idle;
    last_total  = total;

    if(first || d_total == 0)
    {
        return(0.0f);
    }

    return(100.0f * (float)(d_total - d_idle) / (float)d_total);
}

float ViaSystemSensors::ReadRAM()
{
    MEMORYSTATUSEX memory_status;

    memory_status.dwLength = sizeof(memory_status);

    if(!GlobalMemoryStatusEx(&memory_status))
    {
        return(0.0f);
    }

    return((float)memory_status.dwMemoryLoad);
}

/*---------------------------------------------------------*\
| Sum of 3D engine utilization over all processes, the     |
| same number Task Manager shows as "3D"                    |
\*---------------------------------------------------------*/
float ViaSystemSensors::ReadGPU()
{
    if(gpu_failed)
    {
        return(0.0f);
    }

    if(gpu_query == NULL)
    {
        PDH_HQUERY      query;
        PDH_HCOUNTER    counter;

        if(PdhOpenQueryW(NULL, 0, &query) != ERROR_SUCCESS)
        {
            gpu_failed = true;
            return(0.0f);
        }

        if(PdhAddEnglishCounterW(query, GPU_3D_COUNTER_PATH, 0, &counter) != ERROR_SUCCESS)
        {
            VIA_LOG(LL_WARNING, "GPU utilization counters are not available");
            PdhCloseQuery(query);
            gpu_failed = true;
            return(0.0f);
        }

        gpu_query   = query;
        gpu_counter = counter;

        PdhCollectQueryData(query);
        return(0.0f);
    }

    if(PdhCollectQueryData((PDH_HQUERY)gpu_query) != ERROR_SUCCESS)
    {
        return(0.0f);
    }

    DWORD buffer_size = 0;
    DWORD item_count  = 0;

    if(PdhGetFormattedCounterArrayW((PDH_HCOUNTER)gpu_counter, PDH_FMT_DOUBLE, &buffer_size, &item_count, NULL) != PDH_MORE_DATA)
    {
        return(0.0f);
    }

    std::vector<unsigned char> buffer(buffer_size);
    PDH_FMT_COUNTERVALUE_ITEM_W* items = (PDH_FMT_COUNTERVALUE_ITEM_W*)buffer.data();

    if(PdhGetFormattedCounterArrayW((PDH_HCOUNTER)gpu_counter, PDH_FMT_DOUBLE, &buffer_size, &item_count, items) != ERROR_SUCCESS)
    {
        return(0.0f);
    }

    double total = 0.0;

    for(DWORD item_idx = 0; item_idx < item_count; item_idx++)
    {
        if(items[item_idx].FmtValue.CStatus == PDH_CSTATUS_VALID_DATA || items[item_idx].FmtValue.CStatus == PDH_CSTATUS_NEW_DATA)
        {
            total += items[item_idx].FmtValue.doubleValue;
        }
    }

    return(total > 100.0 ? 100.0f : (float)total);
}
