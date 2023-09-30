#include "atompch.h"
#include "ResourceID.h"

#include "Atom/Renderer/RenderGraph/ResourceScheduler.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    ResourceID::ResourceID(const char* name)
        : m_Name(name)
    {
        m_Index = ResourceScheduler::RegisterResource(name);
        ATOM_ENGINE_ASSERT(m_Index != InvalidIndex, fmt::format("Resource with name \"{}\" already exists", name).c_str());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    ResourceID::~ResourceID()
    {
        if (m_Index != InvalidIndex)
            ResourceScheduler::UnregisterResource(m_Index);
    }

}