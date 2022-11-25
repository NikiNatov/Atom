#pragma once

#include "Atom/Core/Core.h"
#include <xhash>

namespace Atom
{
    class UUID
    {
    public:
        UUID();
        UUID(u64 uuid);

        inline operator u64() const { return m_ID; }
    private:
        u64 m_ID;
    };
}

template<>
struct std::hash<Atom::UUID>
{
    std::size_t operator()(const Atom::UUID& uuid) const
    {
        return (uint64_t)uuid;
    }
};