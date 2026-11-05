/*---------------------------------------------------------*\
| ViaWorker.h                                               |
|                                                           |
|   Single background thread for detection and device       |
|   lifecycle (open, handshake, (un)register). Idle unless  |
|   a keyboard is plugged in or removed. Lighting updates   |
|   do not pass through here: they run on OpenRGB's own     |
|   per-device call thread.                                 |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <atomic>
#include <condition_variable>
#include <deque>
#include <functional>
#include <mutex>
#include <thread>

class ViaWorker
{
public:
    ViaWorker();
    ~ViaWorker();

    void    Start();
    void    Post(std::function<void()> task);
    void    Stop();

private:
    std::thread                         thread;
    std::mutex                          queue_lock;
    std::condition_variable             queue_cv;
    std::deque<std::function<void()>>   tasks;
    bool                                exit_requested;
    std::atomic<bool>                   finished;

    void    Run();
};
