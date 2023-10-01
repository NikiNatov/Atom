#include "atompch.h"

#include "ResourceStateTracker.h"
#include "HWResource.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    void ResourceStateTracker::AddGlobalResourceState(const HWResource* resource, ResourceState state)
    {
        if (resource)
        {
            std::lock_guard<std::mutex> lock(ms_GlobalMutex);
            ms_GlobalStates[resource] = state;
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ResourceStateTracker::RemoveGlobalResourceState(const HWResource* resource)
    {
        std::lock_guard<std::mutex> lock(ms_GlobalMutex);

        auto stateEntry = ms_GlobalStates.find(resource);
        if (stateEntry != ms_GlobalStates.end())
        {
            ms_GlobalStates.erase(stateEntry);
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ResourceStateTracker::UpdateGlobalResourceState(const HWResource* resource, ResourceState state, u32 subresource)
    {
        if (resource)
        {
            std::lock_guard<std::mutex> lock(ms_GlobalMutex);

            auto stateEntry = ms_GlobalStates.find(resource);
            if (stateEntry == ms_GlobalStates.end())
            {
                ResourceStateTracker::AddGlobalResourceState(resource, state);
                return;
            }

            stateEntry->second.SetState(state, subresource);
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const ResourceStateTracker::TrackedResourceState& ResourceStateTracker::GetGlobalResourceState(const HWResource* resource)
    {
        std::lock_guard<std::mutex> lock(ms_GlobalMutex);
        ATOM_ENGINE_ASSERT(ms_GlobalStates.find(resource) != ms_GlobalStates.end(), "Resource not registered!");
        return ms_GlobalStates.at(resource);
    }
}
