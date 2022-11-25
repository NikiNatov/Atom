#include "atompch.h"
#include "UUID.h"

#include <random>

namespace Atom
{
    static std::random_device s_RandomDevice;
    static std::mt19937_64 s_Engine(s_RandomDevice());
    static std::uniform_int_distribution<u64> s_UniformDistribution;

    // -----------------------------------------------------------------------------------------------------------------------------
    UUID::UUID()
        : m_ID(s_UniformDistribution(s_Engine))
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    UUID::UUID(u64 uuid)
        : m_ID(uuid)
    {
    }
}
