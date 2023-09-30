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
    class SceneRenderer;

    class RenderGraph
    {
    public:
        RenderGraph(const SceneRenderer& sceneRenderer);
        ~RenderGraph();

        void Build();
        void Execute();
        void Reset();

        template<typename RenderPassType, typename... Args>
        void AddRenderPass(Args&&... args)
        {
            RenderPassID passID = m_Passes.size();
            RenderPassType* pass = new RenderPassType(passID, std::forward<Args>(args)...);
            m_Passes.emplace_back(pass);
        }

        template<typename RenderPassType>
        const RenderPassType* GetRenderPass(RenderPassID passID)
        {
            ATOM_ENGINE_ASSERT(passID < m_Passes.size());
            return dynamic_cast<RenderPassType*>(m_Passes[passID]);
        }

        const Texture* GetFinalOutput() const;
        const Vector<RenderPass*>& GetRenderPasses() const { return m_Passes; }
        const SceneRenderer& GetSceneRenderer() const { return m_SceneRenderer; }
    private:
        struct DependencyGroup
        {
            u32                               GroupIndex;
            Vector<RenderPassID>              Passes;
            Vector<RenderPassID>              PassesPerQueue[u32(CommandQueueType::NumTypes)];
            Set<ResourceID>                   ResourcesReadByMultipleQueues;
            Set<CommandQueueType>             QueuesInvoledInMultiQueueReads;
            Vector<Vector<TransitionBarrier>> TransitionBarriers; // Per-pass transition barriers
            Vector<Vector<UAVBarrier>>        UAVBarriers; // Per-pass UAV barriers
            Vector<TransitionBarrier>         RedirectedTransitionBarriers;
        };

        struct RedirectedTransitionsEvent
        {
            u32 DepGroupIndex;

            Ref<CommandBuffer> CmdBuffer;

            Ref<Fence> SignalFence;
            u64 SignalFenceValue;

            Vector<Ref<Fence>> WaitFences;
            Vector<u64> WaitFenceValues;
        };

        struct RenderPassEvent
        {
            RenderPassID PassID;

            Ref<CommandBuffer> PrePassCmdBuffer;
            Ref<CommandBuffer> PassCmdBuffer;
            Ref<CommandBuffer> PostPassCmdBuffer;

            Ref<Fence> SignalFence;
            u64 SignalFenceValue;

            Vector<Ref<Fence>> WaitFences;
            Vector<u64> WaitFenceValues;
        };

        using RenderGraphEvent = std::variant<RedirectedTransitionsEvent, RenderPassEvent>;
        using RenderGraphEventList = std::list<RenderGraphEvent>;
        using RenderGraphEventListIt = RenderGraphEventList::iterator;

    private:
        void BuildAdjacencyLists();
        void BuildExecutionOrderAndDependencyGroups();
        void BuildSynchronizations();
        void BuildResourceTransitions();
        void BuildRenderPassEvents();
        void BuildRedirectedTransitionsEvents();

        void RecordRedirectedTransitions();
        void RecordRenderPasses();
        void ExecuteCommandLists();

        void DFS(RenderPassID passID, Vector<RenderPassID>& sortedPasses, Vector<bool>& visited, Vector<bool>& nodesOnStack);
        ResourceState GetResourceStateForDependencyGroup(ResourceID resourceID, u32 depGroupIdx, bool isReadOnly);
        
    private:
        const SceneRenderer&           m_SceneRenderer;

        Vector<RenderPass*>            m_Passes;
        Vector<RenderPassID>           m_OrderedPasses;
        Vector<Vector<RenderPassID>>   m_AdjacencyList;
        Vector<DependencyGroup>        m_DependencyGroups;

        Vector<ResourceScheduler>      m_ResourceSchedulers;

        Ref<Fence>                     m_QueueFences[u32(CommandQueueType::NumTypes)];
        RenderGraphEventList           m_RenderGraphEvents[u32(CommandQueueType::NumTypes)];
        Vector<RenderGraphEventListIt> m_RenderPassEvents;
        Vector<RenderGraphEventListIt> m_RedirectedTransitionsEvents;
    };
}