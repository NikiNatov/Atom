#pragma once

#include "Atom/Core/Core.h"

#if defined(ATOM_PLATFORM_WINDOWS)

#include "DirectX12.h"

namespace Atom
{
    class DX12CommandBuffer;

    class DX12ResourceStateTracker
    {
    public:
        class TrackedResourceState
        {
            friend class DX12ResourceStateTracker;
        public:
            TrackedResourceState(D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON)
                : m_State(state)
            {}

            void SetState(D3D12_RESOURCE_STATES state, u32 subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
            {
                if (subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
                {
                    m_State = state;
                    m_SubresourceStates.clear();
                }
                else
                {
                    m_SubresourceStates[subresource] = state;
                }
            }

            D3D12_RESOURCE_STATES GetState(u32 subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
            {
                if (subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
                {
                    return m_State;
                }

                auto state = m_SubresourceStates.find(subresource);
                ATOM_ENGINE_ASSERT(state != m_SubresourceStates.end());

                return state->second;
            }
        private:
            D3D12_RESOURCE_STATES           m_State;
            Map<u32, D3D12_RESOURCE_STATES> m_SubresourceStates;
        };
    public:
        DX12ResourceStateTracker(DX12CommandBuffer& commandBuffer);
        ~DX12ResourceStateTracker();

        void AddTransition(ID3D12Resource* resource, D3D12_RESOURCE_STATES afterState, u32 subresourceIndex = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
        void CommitPendingBarriers();
        void CommitBarriers();
        void UpdateGlobalStates();
        void ClearStates();
    public:
        static void AddGlobalResourceState(ID3D12Resource* resource, D3D12_RESOURCE_STATES state);
        static void RemoveGlobalResourceState(ID3D12Resource* resource);
    private:
        DX12CommandBuffer&                                    m_CommandBuffer;
        Vector<D3D12_RESOURCE_BARRIER>                        m_PendingBarriers;
        Vector<D3D12_RESOURCE_BARRIER>                        m_ResourceBarriers;
        HashMap<ID3D12Resource*, TrackedResourceState>        m_ResourceStates;
    private:
        static HashMap<ID3D12Resource*, TrackedResourceState> ms_GlobalStates;
        static std::mutex                                     ms_GlobalMutex;
    };
}

#endif // ATOM_PLATFORM_WINDOWS