#include "atompch.h"
#include "ThreadPool.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    ThreadPool::ThreadPool(u32 threadCount)
        : m_ThreadCount(threadCount)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    ThreadPool::~ThreadPool()
    {
        Stop();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ThreadPool::Start()
    {
        m_TaskThreads.resize(m_ThreadCount);
        for (auto& thread : m_TaskThreads)
        {
            thread = std::thread(&ThreadPool::TheadLoop, this);
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ThreadPool::Stop()
    {
        {
            std::unique_lock<std::mutex> lock(m_TaskQueueMutex);
            m_ShouldTerminate = true;
        }

        m_TaskQueueCV.notify_all();

        for (auto& thread : m_TaskThreads)
        {
            thread.join();
        }

        m_TaskThreads.clear();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ThreadPool::EnqueueTask(const std::function<void()>& task)
    {
        {
            std::unique_lock<std::mutex> lock(m_TaskQueueMutex);
            m_TaskQueue.push(task);
        }

        m_TaskQueueCV.notify_one();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ThreadPool::TheadLoop()
    {
        while (true)
        {
            std::function<void()> task;

            // Wait and get a task from the queue
            {
                std::unique_lock<std::mutex> lock(m_TaskQueueMutex);
                m_TaskQueueCV.wait(lock, [this]() { return !m_TaskQueue.empty() || m_ShouldTerminate; });

                if (m_ShouldTerminate)
                    return;

                task = std::move(m_TaskQueue.front());
                m_TaskQueue.pop();
            }

            // Execute the task on the current thread
            task();
        }
    }
}
