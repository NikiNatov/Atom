#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Core/DirectX12/DirectX12.h"

#include "Atom/Renderer/ResourceState.h"

namespace Atom
{
    class CommandBuffer;
    class HWResource;
    class ResourceBarrier;
    class TransitionBarrier;

    class ResourceStateTracker
    {
    public:
        class TrackedResourceState
        {
        public:
            TrackedResourceState(ResourceState state = ResourceState::Common)
                : m_State(state)
            {}

            void SetState(ResourceState state, u32 subresource = UINT32_MAX)
            {
                if (subresource == UINT32_MAX)
                {
                    m_State = state;
                    m_SubresourceStates.clear();
                }
                else
                {
                    m_SubresourceStates[subresource] = state;
                }
            }

            ResourceState GetState(u32 subresource = UINT32_MAX) const
            {
                if (subresource == UINT32_MAX)
                {
                    return m_State;
                }

                auto state = m_SubresourceStates.find(subresource);
                if (state != m_SubresourceStates.end())
                {
                    return state->second;
                }

                return m_State;
            }

            inline const Map<u32, ResourceState>& GetSubresourceStates() const { return m_SubresourceStates; }
        private:
            ResourceState           m_State;
            Map<u32, ResourceState> m_SubresourceStates;
        };

        using ResourceStateMap = HashMap<const HWResource*, TrackedResourceState>;

    public:
        static void AddGlobalResourceState(const HWResource* resource, ResourceState state);
        static void RemoveGlobalResourceState(const HWResource* resource);
        static void UpdateGlobalResourceState(const HWResource* resource, ResourceState state, u32 subresource = UINT32_MAX);
        static const TrackedResourceState& GetGlobalResourceState(const HWResource* resource);
    private:
        inline static ResourceStateMap ms_GlobalStates;
        inline static std::mutex       ms_GlobalMutex;
    };
}
