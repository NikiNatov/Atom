#include "atompch.h"
#include "ResourceScheduler.h"

#include "Atom/Scene/SceneRenderer.h"

#include "Atom/Renderer/Device.h"
#include "Atom/Renderer/Renderer.h"

#include "Atom/Renderer/RenderGraph/RenderGraph.h"

namespace Atom
{
    struct FrameConstants
    {
        glm::mat4 ViewMatrix = glm::mat4(1.0f);
        glm::mat4 ProjectionMatrix = glm::mat4(1.0f);
        glm::mat4 InvViewProjMatrix = glm::mat4(1.0f);
        glm::vec3 CameraPosition = glm::vec3(0.0f);
        f32 CameraExposure = 0.5f;
        u32 NumLights = 0;
        f32 p[11]{ 0 };
    };

    // -----------------------------------------------------------------------------------------------------------------------------
    ResourceScheduler::ResourceScheduler(RenderGraph& graph)
        : m_Graph(graph)
    {
        // Create frame constant buffers
        BufferDescription frameCBDesc;
        frameCBDesc.ElementCount = 1;
        frameCBDesc.ElementSize = sizeof(FrameConstants);
        frameCBDesc.IsDynamic = true;

        m_FrameResources.ConstantBuffer = CreateRef<ConstantBuffer>(frameCBDesc, "FrameCB");

        // Create lights structured buffer
        BufferDescription lightsSBDesc;
        lightsSBDesc.ElementCount = 1;
        lightsSBDesc.ElementSize = sizeof(Light);
        lightsSBDesc.IsDynamic = true;

        m_FrameResources.LightsBuffer = CreateRef<StructuredBuffer>(lightsSBDesc, "LightsSB");

        // Create bone transforms structured buffers
        BufferDescription animSBDesc;
        animSBDesc.ElementCount = 1;
        animSBDesc.ElementSize = sizeof(glm::mat4);
        animSBDesc.IsDynamic = true;

        m_FrameResources.BoneTransformsBuffer = CreateRef<StructuredBuffer>(animSBDesc, "BoneTransformsBuffer");
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    ResourceScheduler::~ResourceScheduler()
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ResourceScheduler::BeginScheduling()
    {
        for (auto* resource : m_Resources)
            delete resource;

        m_Resources.clear();
        m_Resources.resize(ms_RegisteredResources.size(), nullptr);

        m_ResourceCurrentStates.clear();
        m_ResourceCurrentStates.resize(ms_RegisteredResources.size(), ResourceState::Common);

        for (auto& inputs : m_PassInputs)
            for (auto* view : inputs)
                delete view;

        m_PassInputs.clear();
        m_PassInputs.resize(m_Graph.GetRenderPasses().size());

        for (auto& outputs : m_PassOutputs)
            for (auto* view : outputs)
                delete view;

        m_PassOutputs.clear();
        m_PassOutputs.resize(m_Graph.GetRenderPasses().size());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ResourceScheduler::EndScheduling()
    {
        for (auto& resource : m_Resources)
            resource->Allocate();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ResourceScheduler::UpdateSceneFrameData(const SceneFrameData& frameData)
    {
        // Update frame constant buffer data
        
        FrameConstants constants;
        constants.ViewMatrix = frameData.CameraConstants.ViewMatrix;
        constants.ProjectionMatrix = frameData.CameraConstants.ProjectionMatrix;
        constants.InvViewProjMatrix = frameData.CameraConstants.InvViewProjMatrix;
        constants.CameraPosition = frameData.CameraConstants.CameraPosition;
        constants.CameraExposure = frameData.CameraConstants.CameraExposure;
        constants.NumLights = frameData.Lights->GetLights().size();

        void* data = m_FrameResources.ConstantBuffer->Map(0, 0);
        memcpy(data, &constants, sizeof(FrameConstants));
        m_FrameResources.ConstantBuffer->Unmap();

        // Update lights structured buffer data
        const auto& lights = frameData.Lights->GetLights();
        if (!lights.empty())
        {
            if (m_FrameResources.LightsBuffer->GetElementCount() != lights.size())
            {
                BufferDescription sbDesc;
                sbDesc.ElementCount = lights.size();
                sbDesc.ElementSize = sizeof(Light);
                sbDesc.IsDynamic = true;

                m_FrameResources.LightsBuffer = CreateRef<StructuredBuffer>(sbDesc, "LightsSB");
            }

            void* lightsData = m_FrameResources.LightsBuffer->Map(0, 0);
            memcpy(lightsData, lights.data(), sizeof(Light) * lights.size());
            m_FrameResources.LightsBuffer->Unmap();
        }

        // Update bone transforms structured buffer data
        if (!frameData.BoneTransforms.empty())
        {
            if (m_FrameResources.BoneTransformsBuffer->GetElementCount() != frameData.BoneTransforms.size())
            {
                BufferDescription animSBDesc;
                animSBDesc.ElementCount = frameData.BoneTransforms.size();
                animSBDesc.ElementSize = sizeof(glm::mat4);
                animSBDesc.IsDynamic = true;

                m_FrameResources.BoneTransformsBuffer = CreateRef<StructuredBuffer>(animSBDesc, "BoneTransformsSB");
            }

            void* boneTransformData = m_FrameResources.BoneTransformsBuffer->Map(0, 0);
            memcpy(boneTransformData, frameData.BoneTransforms.data(), sizeof(glm::mat4) * frameData.BoneTransforms.size());
            m_FrameResources.BoneTransformsBuffer->Unmap();
        }

        // Create resource descriptor tables for the frame
        Ref<TextureCube> envMapAsset = frameData.Lights->GetEnvironmentMap();
        m_FrameResources.EnvironmentMap = envMapAsset ? envMapAsset->GetResource() : Renderer::GetBlackTextureCube();
        m_FrameResources.IrradianceMap = frameData.Lights->GetIrradianceMap() ? frameData.Lights->GetIrradianceMap() : Renderer::GetBlackTextureCube();
        m_FrameResources.BRDFMap = Renderer::GetBRDF();

        D3D12_CPU_DESCRIPTOR_HANDLE frameResourceDescriptors[] = {
            m_FrameResources.EnvironmentMap->GetSRV()->GetDescriptor(),
            m_FrameResources.IrradianceMap->GetSRV()->GetDescriptor(),
            m_FrameResources.BRDFMap->GetSRV()->GetDescriptor(),
            m_FrameResources.LightsBuffer->GetSRV(),
            m_FrameResources.BoneTransformsBuffer->GetSRV()
        };

        m_FrameResources.ResourceTable = Device::Get().GetGPUDescriptorHeap(DescriptorHeapType::ShaderResource)->AllocateTransient(_countof(frameResourceDescriptors));
        Device::Get().CopyDescriptors(m_FrameResources.ResourceTable, _countof(frameResourceDescriptors), frameResourceDescriptors, DescriptorHeapType::ShaderResource);

        // Create sampler descriptor tables for the frame
        Ref<TextureSampler> defaultSampler = Renderer::GetSampler(TextureFilter::Linear, TextureWrap::Clamp);
        Ref<TextureSampler> envMapSampler = envMapAsset ? Renderer::GetSampler(envMapAsset->GetFilter(), envMapAsset->GetWrap()) : defaultSampler;

        D3D12_CPU_DESCRIPTOR_HANDLE frameSamplerDescriptors[] = {
            envMapSampler->GetDescriptor(), // Environment map
            defaultSampler->GetDescriptor(), // Irradiance map sampler
            defaultSampler->GetDescriptor() // BRDF sampler
        };

        m_FrameResources.SamplerTable = Device::Get().GetGPUDescriptorHeap(DescriptorHeapType::Sampler)->AllocateTransient(_countof(frameSamplerDescriptors));
        Device::Get().CopyDescriptors(m_FrameResources.SamplerTable, _countof(frameSamplerDescriptors), frameSamplerDescriptors, DescriptorHeapType::Sampler);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    std::optional<TransitionBarrier> ResourceScheduler::TransitionResource(Resource* resource, ResourceState newState)
    {
        ResourceState currentState = m_ResourceCurrentStates[resource->GetID().GetIndex()];

        if (currentState == newState)
            return {};

        m_ResourceCurrentStates[resource->GetID().GetIndex()] = newState;

        // Check if we can do implicit transition
        if (resource->CanDecayToCommonStateFromState(currentState) && resource->CanPromoteFromCommonStateToState(newState))
            return {};

        return TransitionBarrier(resource->GetHWResource(), currentState, newState);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool ResourceScheduler::IsTransitionSupportedOnQueue(ResourceState beforeState, ResourceState afterState, CommandQueueType queueType)
    {
        auto IsStateSupportedOnQueue = [](ResourceState state, CommandQueueType queue)
        {
            if (queue == CommandQueueType::Copy)
            {
                ResourceState allowedStates = ResourceState::Common |
                    ResourceState::CopyDestination |
                    ResourceState::CopySource;

                return !IsSet(state & ~allowedStates);
            }
            else if (queue == CommandQueueType::Compute)
            {
                ResourceState allowedStates = ResourceState::Common |
                    ResourceState::NonPixelShaderRead |
                    ResourceState::GenericRead |
                    ResourceState::CopyDestination |
                    ResourceState::CopySource |
                    ResourceState::UnorderedAccess |
                    ResourceState::RaytracingAccelerationStructure |
                    ResourceState::ConstantBuffer;

                return !IsSet(state & ~allowedStates);
            }
            else
            {
                return true;
            }
        };

        return IsStateSupportedOnQueue(beforeState, queueType) && IsStateSupportedOnQueue(afterState, queueType);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool ResourceScheduler::IsResourceScheduledForPass(RenderPassID passID, ResourceID resourceID) const
    {
        for (const IResourceView* view : m_PassInputs[passID])
        {
            if (view->GetResourceID() == resourceID)
                return true;
        }

        for (const IResourceView* view : m_PassOutputs[passID])
        {
            if (view->GetResourceID() == resourceID)
                return true;
        }

        return false;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u16 ResourceScheduler::RegisterResource(const char* name)
    {
        for (u16 id = 0; id < ms_RegisteredResources.size(); id++)
            if (strcmp(ms_RegisteredResources[id], name) == 0)
                return ResourceID::InvalidIndex;

        if (!ms_FreeIDs.empty())
        {
            u16 id = ms_FreeIDs.top();
            ms_FreeIDs.pop();
            ms_RegisteredResources[id] = name;
            return id;
        }

        u16 id = ms_RegisteredResources.size();
        ms_RegisteredResources.push_back(name);
        return id;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ResourceScheduler::UnregisterResource(u16 idx)
    {
        ATOM_ENGINE_ASSERT(idx < ms_RegisteredResources.size());
        ms_RegisteredResources[idx] = nullptr;
        ms_FreeIDs.push(idx);
    }

    // -----------------------------------------------------------------------------------------------------------------------------s
    const char* ResourceScheduler::GetResourceName(u16 idx)
    {
        ATOM_ENGINE_ASSERT(idx < ms_RegisteredResources.size());
        return ms_RegisteredResources[idx];
    }

}
