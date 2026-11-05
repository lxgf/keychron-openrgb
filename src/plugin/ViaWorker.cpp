/*---------------------------------------------------------*\
| ViaWorker.cpp                                             |
|                                                           |
|   Single background thread shared by the whole plugin.    |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <QCoreApplication>
#include <QThread>
#include "ViaWorker.h"

ViaWorker::ViaWorker()
{
    exit_requested  = false;
    finished        = false;
}

ViaWorker::~ViaWorker()
{
    Stop();
}

void ViaWorker::Start()
{
    exit_requested  = false;
    finished        = false;
    thread          = std::thread(&ViaWorker::Run, this);
}

void ViaWorker::Post(std::function<void()> task)
{
    {
        std::lock_guard<std::mutex> guard(queue_lock);
        tasks.push_back(std::move(task));
    }

    queue_cv.notify_one();
}

/*---------------------------------------------------------*\
| Runs every queued task, then exits.                       |
|                                                           |
| Device (un)registration ends in a BlockingQueued call to  |
| the OpenRGB GUI thread. When Stop() is called from that   |
| thread (plugin Unload) it keeps processing events while   |
| waiting, otherwise both threads would wait on each other. |
\*---------------------------------------------------------*/
void ViaWorker::Stop()
{
    if(!thread.joinable())
    {
        return;
    }

    {
        std::lock_guard<std::mutex> guard(queue_lock);
        exit_requested = true;
    }

    queue_cv.notify_one();

    QCoreApplication* app = QCoreApplication::instance();

    if(app != NULL && QThread::currentThread() == app->thread())
    {
        while(!finished)
        {
            QCoreApplication::processEvents(QEventLoop::WaitForMoreEvents | QEventLoop::ExcludeUserInputEvents);
        }
    }

    thread.join();
}

void ViaWorker::Run()
{
    while(true)
    {
        std::function<void()> task;

        {
            std::unique_lock<std::mutex> lock(queue_lock);

            queue_cv.wait(lock, [this]{ return(exit_requested || !tasks.empty()); });

            if(tasks.empty())
            {
                break;
            }

            task = std::move(tasks.front());
            tasks.pop_front();
        }

        task();
    }

    finished = true;

    /*-----------------------------------------------------*\
    | Wake a GUI thread waiting in Stop()                   |
    \*-----------------------------------------------------*/
    QCoreApplication* app = QCoreApplication::instance();

    if(app != NULL)
    {
        QMetaObject::invokeMethod(app, []{}, Qt::QueuedConnection);
    }
}
