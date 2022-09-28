#pragma once

#include "Atom/Core/Core.h"

namespace Atom
{
    template<typename Type>
    class ThreadSafeQueue
    {
    public:
        ThreadSafeQueue() = default;

        ThreadSafeQueue(const ThreadSafeQueue<Type>& rhs)
        {
            std::lock_guard<std::mutex> lock(rhs.m_Mutex);
            m_Queue = rhs.m_Queue;
        }

        ~ThreadSafeQueue() = default;

        void Push(const Type& value)
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            m_Queue.push(value);
        }

        void Push(Type&& value)
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            m_Queue.push(value);
        }

        template<typename... Args>
        void Emplace(Args&&... args)
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            m_Queue.emplace(std::forward<Args>(args)...);
        }

        Type Pop()
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            Type value = std::move(m_Queue.front());
            m_Queue.pop();
            return value;
        }

        bool Empty()
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            return m_Queue.empty();
        }

        u32 Size()
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            return m_Queue.size();
        }
    private:
        std::mutex  m_Mutex;
        Queue<Type> m_Queue;
    };
}