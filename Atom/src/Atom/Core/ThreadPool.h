#pragma once

#include "Atom/Core/Core.h"

namespace Atom
{
    class ThreadPool
    {
    public:
        ThreadPool(u32 threadCount = std::thread::hardware_concurrency());
        ~ThreadPool();

        void Start();
        void Stop();
        void EnqueueTask(const std::function<void()>& task);
        inline u32 GetThreadCount() const { return m_ThreadCount; }
    private:
        void TheadLoop();
    private:
        u32                          m_ThreadCount;
        Vector<std::thread>          m_TaskThreads;
        Queue<std::function<void()>> m_TaskQueue;
        std::condition_variable      m_TaskQueueCV;
        std::mutex                   m_TaskQueueMutex;
        bool                         m_ShouldTerminate = false;
    };
}