#include "atompch.h"
#include "Resource.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    Resource::Resource(const ResourceID& id, bool external)
        : m_ID(id), m_External(external), m_ProducerPassID(UINT16_MAX)
    {
    }
}
