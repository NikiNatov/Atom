#pragma once

#include "Atom/Core/Core.h"

namespace Atom
{
    class HashBuilder
    {
    public:
        HashBuilder() = default;

        template <class T>
        inline void AddToHash(const T& v) { m_Hash ^= std::hash<T>{}(v) + 0x9e3779b9 + (m_Hash << 6) + (m_Hash >> 2); }

        inline u64 GetHash() const { return m_Hash; }
    private:
        u64 m_Hash = 0;
    };
}