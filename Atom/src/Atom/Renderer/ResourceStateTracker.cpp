#include "atompch.h"

#include "ResourceStateTracker.h"
#include "CommandBuffer.h"

namespace Atom
{
    HashMap<ID3D12Resource*, ResourceStateTracker::TrackedResourceState> ResourceStateTracker::ms_GlobalStates;
    std::mutex ResourceStateTracker::ms_GlobalMutex;

    // -----------------------------------------------------------------------------------------------------------------------------
    ResourceStateTracker::ResourceStateTracker(CommandBuffer& commandBuffer)
        : m_CommandBuffer(commandBuffer)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    ResourceStateTracker::~ResourceStateTracker()
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ResourceStateTracker::AddTransition(ID3D12Resource* resource, D3D12_RESOURCE_STATES afterState, u32 subresourceIndex)
    {
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Transition.pResource = resource;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
        barrier.Transition.StateAfter = afterState;
        barrier.Transition.Subresource = subresourceIndex;

        auto stateEntry = m_ResourceStates.find(resource);
        if (stateEntry != m_ResourceStates.end())
        {
            // Resource was already used on that command list so get the current state
            TrackedResourceState& currentTrackedState = stateEntry->second;

            if (subresourceIndex == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES && !currentTrackedState.m_SubresourceStates.empty())
            {
                for (auto& [subresource, beforeState] : currentTrackedState.m_SubresourceStates)
                {
                    if (beforeState != afterState)
                    {
                        D3D12_RESOURCE_BARRIER subresourceBarrier = barrier;
                        subresourceBarrier.Transition.StateBefore = beforeState;
                        subresourceBarrier.Transition.Subresource = subresource;
                        m_ResourceBarriers.push_back(subresourceBarrier);
                    }
                }
            }
            else
            {
                auto beforeState = currentTrackedState.GetState(subresourceIndex);
                if (beforeState != afterState)
                {
                    barrier.Transition.StateBefore = beforeState;
                    m_ResourceBarriers.push_back(barrier);
                }
            }
        }
        else
        {
            // Resource is being used for the first time on this command list so add transition to pending barriers
            m_PendingBarriers.push_back(barrier);
        }

        // Update the current tracked resource state
        m_ResourceStates[resource].SetState(afterState, subresourceIndex);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ResourceStateTracker::CommitPendingBarriers()
    {
        //std::lock_guard<std::mutex> lock(ms_GlobalMutex);

        Vector<D3D12_RESOURCE_BARRIER> resolvedBarriers;
        resolvedBarriers.reserve(m_PendingBarriers.size());

        // Resolve before state
        for (auto& pendingBarrier : m_PendingBarriers)
        {
            ATOM_ENGINE_ASSERT(ms_GlobalStates.find(pendingBarrier.Transition.pResource) != ms_GlobalStates.end(), "Resource not registered!");
            TrackedResourceState& globalTrackedState = ms_GlobalStates[pendingBarrier.Transition.pResource];

            if (pendingBarrier.Transition.Subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES && !globalTrackedState.m_SubresourceStates.empty())
            {
                for (auto& [subresource, beforeState] : globalTrackedState.m_SubresourceStates)
                {
                    if (beforeState != pendingBarrier.Transition.StateAfter)
                    {
                        D3D12_RESOURCE_BARRIER subresourceBarrier = pendingBarrier;
                        subresourceBarrier.Transition.StateBefore = beforeState;
                        subresourceBarrier.Transition.Subresource = subresource;
                        resolvedBarriers.push_back(subresourceBarrier);
                    }
                }
            }
            else
            {
                auto beforeState = globalTrackedState.GetState(pendingBarrier.Transition.Subresource);
                if (beforeState != pendingBarrier.Transition.StateAfter)
                {
                    pendingBarrier.Transition.StateBefore = beforeState;
                    resolvedBarriers.push_back(pendingBarrier);
                }
            }
        }

        u32 barrierCount = resolvedBarriers.size();
        if (barrierCount)
        {
            m_CommandBuffer.GetPendingCommandList()->ResourceBarrier(barrierCount, resolvedBarriers.data());
        }

        m_PendingBarriers.clear();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ResourceStateTracker::CommitBarriers()
    {
        u32 barrierCount = m_ResourceBarriers.size();
        if (barrierCount)
        {
            m_CommandBuffer.GetCommandList()->ResourceBarrier(barrierCount, m_ResourceBarriers.data());
        }
        m_ResourceBarriers.clear();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ResourceStateTracker::UpdateGlobalStates()
    {
        std::lock_guard<std::mutex> lock(ms_GlobalMutex);

        for (auto& [resource, trackedState] : m_ResourceStates)
        {
            ms_GlobalStates[resource] = trackedState;
        }

        m_ResourceStates.clear();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ResourceStateTracker::ClearStates()
    {
        m_PendingBarriers.clear();
        m_ResourceBarriers.clear();
        m_ResourceStates.clear();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ResourceStateTracker::AddGlobalResourceState(ID3D12Resource* resource, D3D12_RESOURCE_STATES state)
    {
        if (resource)
        {
            std::lock_guard<std::mutex> lock(ms_GlobalMutex);
            ms_GlobalStates[resource] = state;
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ResourceStateTracker::RemoveGlobalResourceState(ID3D12Resource* resource)
    {
        std::lock_guard<std::mutex> lock(ms_GlobalMutex);

        auto stateEntry = ms_GlobalStates.find(resource);
        if (stateEntry != ms_GlobalStates.end())
        {
            ms_GlobalStates.erase(stateEntry);
        }
    }
}
