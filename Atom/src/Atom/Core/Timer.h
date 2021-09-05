#pragma once

#include "Core.h"
#include "Timestep.h"
#include <chrono>

namespace Atom
{
    class Timer
    {
    public:
        Timer();
        ~Timer();

        void Reset();
        void Stop();

        inline Timestep GetElapsedTime() const { return Timestep(m_ElapsedTime); }
    private:
        std::chrono::time_point<std::chrono::steady_clock> m_Start;
        f32                                                m_ElapsedTime = 0.0f;
    };
}