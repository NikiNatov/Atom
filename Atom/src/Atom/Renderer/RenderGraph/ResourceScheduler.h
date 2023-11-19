#pragma once

#include "Atom/Core/Core.h"

#include "Atom/Renderer/ResourceBarrier.h"
#include "Atom/Renderer/Pipeline.h"

#include "Atom/Renderer/RenderGraph/Resource.h"
#include "Atom/Renderer/RenderGraph/RenderPass.h"
#include "Atom/Renderer/RenderGraph/ResourceView.h"

#include <optional>

namespace Atom
{
    class ResourceScheduler
    {
    public:
        ResourceScheduler();
        ~ResourceScheduler();

        void BeginScheduling(u32 numRenderPasses);
        void EndScheduling();
        std::optional<TransitionBarrier> TransitionResource(Resource* resource, ResourceState newState);
        bool IsTransitionSupportedOnQueue(ResourceState beforeState, ResourceState afterState, CommandQueueType queueType);
        bool IsResourceScheduledForPass(RenderPassID passID, const ResourceID& resourceID) const;

        void AssignPipelineToPass(RenderPassID passID, const GraphicsPipelineDescription& description);
        void AssignPipelineToPass(RenderPassID passID, const ComputePipelineDescription& description);

        template<typename ResourceType, typename ResourceIDType>
        ResourceType* CreateResource(const ResourceIDType& id, const typename ResourceType::ResourceDescType& description)
        {
            u16 idIndex = id.GetIndex();
            ATOM_ENGINE_ASSERT(m_Resources[idIndex] == nullptr, "Resource already created!");
            ResourceType* resource = new ResourceType(id, description);
            m_Resources[idIndex] = resource;
            m_ResourceCurrentStates[idIndex] = resource->GetInitialState();
            return resource;
        }

        template<typename ResourceType, typename ResourceIDType>
        ResourceType* CreateResource(const ResourceIDType& id, typename ResourceType::HWResourceType* externalResource)
        {
            u16 idIndex = id.GetIndex();
            ATOM_ENGINE_ASSERT(m_Resources[idIndex] == nullptr, "Resource already created!");
            ResourceType* resource = new ResourceType(id, externalResource);
            m_Resources[idIndex] = resource;
            m_ResourceCurrentStates[idIndex] = resource->GetInitialState();
            return resource;
        }

        Resource* GetResource(const ResourceID& id) const
        {
            for (auto& resource : m_Resources)
            {
                if (resource && resource->GetID() == id)
                    return resource;
            }

            return nullptr;
        }

        template<typename ViewClass>
        void CreateResourceView(RenderPassID passID, const ResourceID& resourceID)
        {
            ATOM_ENGINE_ASSERT(!IsResourceScheduledForPass(passID, resourceID), fmt::format("Resource %s already scheduled!", resourceID.GetName()).c_str());

            if constexpr (std::is_same_v<ViewClass, TextureSRV> || std::is_same_v<ViewClass, SurfaceSRV> || std::is_same_v<ViewClass, SurfaceDSV_RO>)
            {
                m_PassInputs[passID].push_back(new ResourceView<ViewClass>(resourceID, *this));
            }
            else
            {
                m_PassOutputs[passID].push_back(new ResourceView<ViewClass>(resourceID, *this));
            }
        }

        inline const Vector<Resource*>& GetResources() const { return m_Resources; }
        inline const Vector<IResourceView*>& GetPassInputs(RenderPassID passID) const { return m_PassInputs[passID]; }
        inline const Vector<IResourceView*>& GetPassOutputs(RenderPassID passID) const { return m_PassOutputs[passID]; }
        inline const Pipeline* GetPassPipeline(RenderPassID passID) const { return m_PassPipelines[passID]; }
    public:
        static u16 RegisterResource(const char* name);
        static void UnregisterResource(u16 idx);
        static const char* GetResourceName(u16 idx);
    private:
        Vector<Resource*>              m_Resources;
        Vector<ResourceState>          m_ResourceCurrentStates;

        Vector<Vector<IResourceView*>> m_PassInputs;
        Vector<Vector<IResourceView*>> m_PassOutputs;
        Vector<Pipeline*>              m_PassPipelines;
    private:
        inline static Vector<const char*> ms_RegisteredResources;
        inline static Stack<u16>          ms_FreeIDs;
    };
}