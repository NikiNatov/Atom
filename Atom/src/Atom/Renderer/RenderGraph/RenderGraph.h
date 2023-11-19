#pragma once

#include "Atom/Core/Core.h"

#include "Atom/Renderer/CommandQueue.h"
#include "Atom/Renderer/ResourceState.h"
#include "Atom/Renderer/Fence.h"

#include "Atom/Renderer/RenderGraph/RenderPass.h"
#include "Atom/Renderer/RenderGraph/ResourceScheduler.h"

#include <variant>

namespace Atom
{
    class RenderGraph
    {
    public:
        struct FenceSignal
        {
            Ref<Fence> Fence = nullptr;
            u64 FenceValue;
        };

        struct RedirectedTransitionsEvent
        {
            u32 DepGroupIndex;
            const Vector<TransitionBarrier>* RedirectedTransitionBarriers = nullptr; // Pointer to the redirected transitions list of the dependency group

            Ref<CommandBuffer> CmdBuffer;
            FenceSignal Signal;
            Vector<FenceSignal*> SignalsToWait;
        };

        struct RenderPassEvent
        {
            RenderPass* RenderPass = nullptr; // Pointer to the pass in the m_Passes array
            const Vector<TransitionBarrier>* TransitionBarriers = nullptr; // Pointer to the transitions list of the dependency group
            const Vector<UAVBarrier>* UAVBarriers = nullptr; // Pointer to the UAV barriers list of the dependency group

            Ref<CommandBuffer> CmdBuffer;
            FenceSignal Signal;
            Vector<FenceSignal*> SignalsToWait;
        };

        using RenderGraphEvent = std::variant<RedirectedTransitionsEvent, RenderPassEvent>;
        using RenderGraphEventList = std::list<RenderGraphEvent>;
        using RenderGraphEventListIt = RenderGraphEventList::iterator;
    public:
        RenderGraph();
        ~RenderGraph();

        void Build(ResourceScheduler& resourceScheduler);
        void Reset();

        template<typename RenderPassType, typename... Args>
        void AddRenderPass(Args&&... args)
        {
            RenderPassID passID = m_Passes.size();
            RenderPassType* pass = new RenderPassType(passID, std::forward<Args>(args)...);
            m_Passes.emplace_back(pass);
        }

        inline const RenderPass* GetRenderPass(RenderPassID passID) const { return m_Passes[passID]; }
        inline const Vector<RenderPass*>& GetRenderPasses() const { return m_Passes; }
        inline const Vector<RenderPassID>& GetOrderedPasses() const { return m_OrderedPasses; }
        inline const RenderGraphEventList& GetRenderGraphEvents(CommandQueueType queueType) const { return m_RenderGraphEvents[(u32)queueType]; }

    private:
        void BuildAdjacencyLists(ResourceScheduler& resourceScheduler);
        void BuildExecutionOrderAndDependencyGroups(ResourceScheduler& resourceScheduler);
        void BuildSynchronizations();
        void BuildResourceTransitions(ResourceScheduler& resourceScheduler);
        void BuildRenderPassEvents();
        void BuildRedirectedTransitionsEvents();
        void BuildEventFenceValues();

        void DFS(RenderPassID passID, Vector<RenderPassID>& sortedPasses, Vector<bool>& visited, Vector<bool>& nodesOnStack);
        ResourceState GetResourceStateForDependencyGroup(const ResourceID& resourceID, u32 depGroupIdx, bool isReadOnly, ResourceScheduler& resourceScheduler);

    private:
        struct DependencyGroup
        {
            u32                               GroupIndex;
            Vector<RenderPassID>              Passes;
            Vector<RenderPassID>              PassesPerQueue[u32(CommandQueueType::NumTypes)];
            Set<u16>                          ResourcesReadByMultipleQueues;
            Set<CommandQueueType>             QueuesInvoledInMultiQueueReads;
            Vector<Vector<TransitionBarrier>> TransitionBarriers; // Per-pass transition barriers
            Vector<Vector<UAVBarrier>>        UAVBarriers; // Per-pass UAV barriers
            Vector<TransitionBarrier>         RedirectedTransitionBarriers;
        };

        Vector<RenderPass*>            m_Passes;
        Vector<RenderPassID>           m_OrderedPasses;
        Vector<Vector<RenderPassID>>   m_AdjacencyList;
        Vector<DependencyGroup>        m_DependencyGroups;

        Ref<Fence>                     m_QueueFences[u32(CommandQueueType::NumTypes)];
        RenderGraphEventList           m_RenderGraphEvents[u32(CommandQueueType::NumTypes)];
        Vector<RenderGraphEventListIt> m_RenderPassEvents;
        Vector<RenderGraphEventListIt> m_RedirectedTransitionsEvents;
    };
}