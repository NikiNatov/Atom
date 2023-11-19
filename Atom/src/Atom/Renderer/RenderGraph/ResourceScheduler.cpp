#include "atompch.h"
#include "ResourceScheduler.h"

#include "Atom/Renderer/PipelineLibrary.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    ResourceScheduler::ResourceScheduler()
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    ResourceScheduler::~ResourceScheduler()
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ResourceScheduler::BeginScheduling(u32 numRenderPasses)
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
        m_PassInputs.resize(numRenderPasses);

        for (auto& outputs : m_PassOutputs)
            for (auto* view : outputs)
                delete view;

        m_PassOutputs.clear();
        m_PassOutputs.resize(numRenderPasses);

        m_PassPipelines.clear();
        m_PassPipelines.resize(numRenderPasses);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ResourceScheduler::EndScheduling()
    {
        for (auto& resource : m_Resources)
            resource->Allocate();
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

                return !IsSet(state, ~allowedStates);
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
                    ResourceState::VertexConstantBuffer;

                return !IsSet(state, ~allowedStates);
            }
            else
            {
                return true;
            }
        };

        return IsStateSupportedOnQueue(beforeState, queueType) && IsStateSupportedOnQueue(afterState, queueType);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool ResourceScheduler::IsResourceScheduledForPass(RenderPassID passID, const ResourceID& resourceID) const
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
    void ResourceScheduler::AssignPipelineToPass(RenderPassID passID, const GraphicsPipelineDescription& description)
    {
        m_PassPipelines[passID] = PipelineLibrary::Get().LoadGraphicsPipeline(description).get();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ResourceScheduler::AssignPipelineToPass(RenderPassID passID, const ComputePipelineDescription& description)
    {
        m_PassPipelines[passID] = PipelineLibrary::Get().LoadComputePipeline(description).get();
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
