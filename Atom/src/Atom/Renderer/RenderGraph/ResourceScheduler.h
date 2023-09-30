#pragma once

#include "Atom/Core/Core.h"

#include "Atom/Renderer/DescriptorHeap.h"
#include "Atom/Renderer/Buffer.h"
#include "Atom/Renderer/Texture.h"
#include "Atom/Renderer/TextureSampler.h"
#include "Atom/Renderer/ResourceBarrier.h"

#include "Atom/Renderer/RenderGraph/Resource.h"
#include "Atom/Renderer/RenderGraph/RenderPass.h"
#include "Atom/Renderer/RenderGraph/ResourceView.h"

#include <optional>

namespace Atom
{
    class RenderGraph;
    class SceneFrameData;

    class ResourceScheduler
    {
    public:
        struct FrameResources
        {
            // CBVs
            Ref<ConstantBuffer>   ConstantBuffer;

            // SRVs
            Ref<StructuredBuffer> BoneTransformsBuffer;
            Ref<StructuredBuffer> LightsBuffer;
            Ref<Texture>          EnvironmentMap;
            Ref<Texture>          IrradianceMap;
            Ref<Texture>          BRDFMap;
            DescriptorAllocation  ResourceTable;

            // Samplers
            Ref<TextureSampler>   EnvironmentMapSampler;
            Ref<TextureSampler>   IrradianceMapSampler;
            Ref<TextureSampler>   BRDFMapSampler;
            DescriptorAllocation  SamplerTable;
        };
    public:
        ResourceScheduler(RenderGraph& graph);
        ~ResourceScheduler();

        void BeginScheduling();
        void EndScheduling();
        void UpdateSceneFrameData(const SceneFrameData& frameData);
        std::optional<TransitionBarrier> TransitionResource(Resource* resource, ResourceState newState);
        bool IsTransitionSupportedOnQueue(ResourceState beforeState, ResourceState afterState, CommandQueueType queueType);
        bool IsResourceScheduledForPass(RenderPassID passID, ResourceID resourceID) const;

        template<typename ResourceType, typename ResourceIDType>
        ResourceType* CreateResource(ResourceIDType id, const typename ResourceType::ResourceDescType& description)
        {
            u16 idIndex = id.GetIndex();
            ATOM_ENGINE_ASSERT(m_Resources[idIndex] == nullptr, "Resource already created!");
            ResourceType* resource = new ResourceType(id, description);
            m_Resources[idIndex] = resource;
            m_ResourceCurrentStates[idIndex] = resource->GetInitialState();
            return resource;
        }

        template<typename ResourceType, typename ResourceIDType>
        ResourceType* CreateResource(ResourceIDType id, typename ResourceType::HWResourceType* externalResource)
        {
            u16 idIndex = id.GetIndex();
            ATOM_ENGINE_ASSERT(m_Resources[idIndex] == nullptr, "Resource already created!");
            ResourceType* resource = new ResourceType(id, externalResource);
            m_Resources[idIndex] = resource;
            m_ResourceCurrentStates[idIndex] = resource->GetInitialState();
            return resource;
        }

        Resource* GetResource(ResourceID id) const
        {
            for (auto& resource : m_Resources)
            {
                if (resource && resource->GetID() == id)
                    return resource;
            }

            return nullptr;
        }

        template<typename ViewClass>
        void CreateResourceView(RenderPassID passID, ResourceID resourceID)
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

        inline const FrameResources& GetFrameResources() const { return m_FrameResources; }
        inline const Vector<Resource*>& GetResources() const { return m_Resources; }
        inline const Vector<IResourceView*>& GetPassInputs(RenderPassID passID) const { return m_PassInputs[passID]; }
        inline const Vector<IResourceView*>& GetPassOutputs(RenderPassID passID) const { return m_PassOutputs[passID]; }
    public:
        static u16 RegisterResource(const char* name);
        static void UnregisterResource(u16 idx);
        static const char* GetResourceName(u16 idx);
    private:
        RenderGraph&                   m_Graph;

        FrameResources                 m_FrameResources;
        Vector<Resource*>              m_Resources;
        Vector<ResourceState>          m_ResourceCurrentStates;

        Vector<Vector<IResourceView*>> m_PassInputs;
        Vector<Vector<IResourceView*>> m_PassOutputs;
    private:
        inline static Vector<const char*> ms_RegisteredResources;
        inline static Stack<u16>          ms_FreeIDs;
    };
}