#pragma once

#include "Core.h"

namespace Atom
{
    class Timestep
    {
    public:
        Timestep(f32 time = 0.0f)
            : m_Time(time)
        {}

        f32 GetSeconds() const { return m_Time / 1000.0f; }
        f32 GetMilliseconds() const { return m_Time; }

        operator f32() const { return GetSeconds(); }
    private:
        f32 m_Time;
    };
}