#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Renderer/CommandQueue.h"
#include "Atom/Renderer/RenderGraph/ResourceID.h"
#include "Atom/Renderer/RenderGraph/Resource.h"
#include "Atom/Renderer/RenderGraph/RenderPass.h"

namespace Atom
{
    class RenderGraph
    {
    public:
        struct DependencyGroup
        {
            u32                  GroupIndex;
            Vector<RenderPassID> Passes;
            Vector<RenderPassID> PassesPerQueue[u32(CommandQueueType::NumTypes)];
        };
    public:
        RenderGraph();

        void Build();
        void Execute();
        void Reset();

        void AddRenderPass(const String& name, CommandQueueType queueType, const RenderPass::BuildCallback& buildFn, const RenderPass::ExecuteCallback& executeFn)
        {
            m_Passes.emplace_back(m_Passes.size(), name, queueType, buildFn, executeFn);
        }

        const RenderPass& GetRenderPass(RenderPassID passID)
        {
            ATOM_ENGINE_ASSERT(passID < m_Passes.size());
            return m_Passes[passID];
        }

        template<typename ResourceType, typename ResourceIDType, typename DescType>
        const Ref<ResourceType>& CreateResource(ResourceIDType id, const DescType& description)
        {
            ATOM_ENGINE_ASSERT(m_Resources[id.GetIndex()] == nullptr, "Resource already created!");
            Ref<ResourceType> resource = CreateRef<ResourceType>(id, description);
            m_Resources[id.GetIndex()] = resource;
            return resource;
        }

        template<typename ResourceType, typename ResourceIDType, typename ExternalResourceType>
        const Ref<ResourceType>& CreateResource(ResourceIDType id, const ExternalResourceType* externalResource)
        {
            ATOM_ENGINE_ASSERT(m_Resources[id.GetIndex()] == nullptr, "Resource already created!");
            Ref<ResourceType> resource = CreateRef<ResourceType>(id, externalResource);
            m_Resources[id.GetIndex()] = resource;
            return resource;
        }

        const Ref<Resource>& GetResource(ResourceID id) const;
    private:
        void BuildAdjacencyLists();
        void BuildExecutionOrder();
        void BuildDependencyGroups();
        void BuildResourceLifetimes();
        void BuildSynchronizations();
        void DFS(RenderPassID passID, Vector<bool>& visited, Vector<bool>& nodesOnStack);
    private:
        struct ResourceLifetime
        {
            RenderPassID FirstPass = UINT16_MAX;
            RenderPassID EndPass = UINT16_MAX;
        };

        Vector<RenderPass>                 m_Passes;
        Vector<RenderPassID>               m_SortedPasses;
        Vector<RenderPassID>               m_FinalExecutionOrderPasses;
        Vector<Vector<RenderPassID>>       m_AdjacencyList;
        Vector<DependencyGroup>            m_DependencyGroups;

        Vector<Ref<Resource>>              m_Resources;
        Vector<ResourceLifetime>           m_ResourceLifetimes;
    };
}