#include "atompch.h"
#include "ResourceID.h"

namespace Atom
{
    // ---------------------------------------------------- ResourceIDRegistry -----------------------------------------------------
    // -----------------------------------------------------------------------------------------------------------------------------
    u16 ResourceIDRegistry::Register(const char* name)
    {
        for (u16 id = 0; id < ms_Resources.size(); id++)
            if (strcmp(ms_Resources[id], name) == 0)
                return ResourceID::InvalidID;

        if (!ms_FreeIDs.empty())
        {
            u16 id = ms_FreeIDs.top();
            ms_FreeIDs.pop();
            ms_Resources[id] = name;
            return id;
        }

        u16 id = ms_Resources.size();
        ms_Resources.push_back(name);
        return id;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ResourceIDRegistry::Unregister(u16 idx)
    {
        ATOM_ENGINE_ASSERT(idx < ms_Resources.size());
        ms_Resources[idx] = nullptr;
        ms_FreeIDs.push(idx);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const char* ResourceIDRegistry::GetName(u16 idx)
    {
        ATOM_ENGINE_ASSERT(idx < ms_Resources.size());
        return ms_Resources[idx];
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 ResourceIDRegistry::GetResourceCount()
    {
        return ms_Resources.size();
    }

    // -------------------------------------------------------- ResourceID ---------------------------------------------------------
    // -----------------------------------------------------------------------------------------------------------------------------
    ResourceID::ResourceID(const char* name)
    {
        m_Index = ResourceIDRegistry::Register(name);
        ATOM_ENGINE_ASSERT(m_Index != InvalidID, fmt::format("Resource with name \"{}\" already exists", name).c_str());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    ResourceID::~ResourceID()
    {
        if (m_Index != InvalidID)
            ResourceIDRegistry::Unregister(m_Index);
    }

}