#include "atompch.h"
#include "RenderGraph.h"

#include "Atom/Renderer/ResourceBarrier.h"
#include "Atom/Renderer/Device.h"
#include "Atom/Renderer/Renderer.h"

#include "Atom/Renderer/RenderGraph/RenderPassBuilder.h"
#include "Atom/Renderer/RenderGraph/RenderPassContext.h"
#include "Atom/Renderer/RenderGraph/ResourceView.h"

#include "Atom/Scene/SceneRenderer.h"


#include <pix3.h>

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    RenderGraph::RenderGraph(const SceneRenderer& sceneRenderer)
        : m_SceneRenderer(sceneRenderer)
    {
        m_QueueFences[u32(CommandQueueType::Graphics)] = CreateRef<Fence>("Render Graph GfxQueue Fence");
        m_QueueFences[u32(CommandQueueType::Compute)] = CreateRef<Fence>("Render Graph ComputeQueue Fence");
        m_QueueFences[u32(CommandQueueType::Copy)] = CreateRef<Fence>("Render Graph CopyQueue Fence");

        m_ResourceSchedulers.resize(Renderer::GetFramesInFlight(), ResourceScheduler(*this));
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    RenderGraph::~RenderGraph()
    {
        Reset();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderGraph::Build()
    {
        BuildAdjacencyLists();
        BuildExecutionOrderAndDependencyGroups();
        BuildSynchronizations();
        BuildResourceTransitions();
        BuildRenderPassEvents();
        BuildRedirectedTransitionsEvents();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderGraph::Execute()
    {
        m_ResourceSchedulers[Renderer::GetCurrentFrameIndex()].UpdateSceneFrameData(m_SceneRenderer.GetSceneFrameData());

        RecordRedirectedTransitions();
        RecordRenderPasses();
        ExecuteCommandLists();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderGraph::Reset()
    {
        for (auto* pass : m_Passes)
            delete pass;

        m_Passes.clear();
        m_OrderedPasses.clear();
        m_AdjacencyList.clear();
        m_DependencyGroups.clear();
        m_RenderPassEvents.clear();
        m_RedirectedTransitionsEvents.clear();

        for (u32 queueIdx = 0; queueIdx < u32(CommandQueueType::NumTypes); queueIdx++)
            m_RenderGraphEvents[queueIdx].clear();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const Texture* RenderGraph::GetFinalOutput() const
    {
        const ResourceScheduler& resourceScheduler = m_ResourceSchedulers[Renderer::GetCurrentFrameIndex()];
        const IResourceView* finalOutputView = resourceScheduler.GetPassOutputs(m_OrderedPasses.back())[0];
        HWResource* finalOutputTexture = resourceScheduler.GetResource(finalOutputView->GetResourceID())->GetHWResource();

        return dynamic_cast<Texture*>(finalOutputTexture);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderGraph::BuildAdjacencyLists()
    {
        // Build pass resources
        u32 frameIdx = Renderer::GetCurrentFrameIndex();
        ResourceScheduler& resourceScheduler = m_ResourceSchedulers[frameIdx];

        resourceScheduler.BeginScheduling();

        for (auto& pass : m_Passes)
        {
            RenderPassBuilder passBuilder(pass->m_ID, resourceScheduler);
            pass->Build(passBuilder);
        }

        resourceScheduler.EndScheduling();

        // Create adjacency lists
        m_AdjacencyList.resize(m_Passes.size());
        for (auto& pass : m_Passes)
        {
            for (auto& otherPass : m_Passes)
            {
                if (otherPass->m_ID == pass->m_ID)
                    continue;

                for (auto& input : resourceScheduler.GetPassInputs(otherPass->m_ID))
                {
                    if (resourceScheduler.GetResource(input->GetResourceID())->GetProducerPassID() == pass->m_ID)
                    {
                        m_AdjacencyList[pass->m_ID].push_back(otherPass->m_ID);

                        if (pass->m_QueueType != otherPass->m_QueueType)
                        {
                            pass->m_SignalRequired = true;
                            otherPass->m_SyncPasses.insert(pass->m_ID);
                        }

                        break;
                    }
                }
            }
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderGraph::BuildExecutionOrderAndDependencyGroups()
    {
        Vector<bool> visited(m_Passes.size(), false);
        Vector<bool> nodesOnStack(m_Passes.size(), false);

        Vector<RenderPassID> sortedPasses;
        sortedPasses.reserve(m_Passes.size());

        for (RenderPassID passID = 0; passID < m_Passes.size(); passID++)
            if (!visited[passID])
                DFS(passID, sortedPasses, visited, nodesOnStack);

        std::reverse(sortedPasses.begin(), sortedPasses.end());

        // Get max distances for each node from the root
        Vector<u32> maxDistances(m_Passes.size(), 0);

        u32 numDependencyGroups = 1;
        for (RenderPassID passID : sortedPasses)
        {
            for (RenderPassID neighbourPassID : m_AdjacencyList[passID])
            {
                maxDistances[neighbourPassID] = std::max(maxDistances[neighbourPassID], maxDistances[passID] + 1);
                numDependencyGroups = std::max(numDependencyGroups, maxDistances[neighbourPassID] + 1);
            }
        }

        // Add every pass node to its corresponding dependency group
        m_DependencyGroups.resize(numDependencyGroups);
        for (RenderPass* pass : m_Passes)
        {
            pass->m_DependencyGroupIndex = maxDistances[pass->m_ID];
            m_DependencyGroups[pass->m_DependencyGroupIndex].GroupIndex = pass->m_DependencyGroupIndex;
            m_DependencyGroups[pass->m_DependencyGroupIndex].Passes.push_back(pass->m_ID);
            m_DependencyGroups[pass->m_DependencyGroupIndex].PassesPerQueue[u32(pass->m_QueueType)].push_back(pass->m_ID);
        }

        // Set execution indices and build final execution order of the passes
        m_OrderedPasses.resize(m_Passes.size());

        u32 passCountPerQueue[u32(CommandQueueType::NumTypes)] = { 0, 0, 0 };
        RenderPassID prevPassPerQueue[u32(CommandQueueType::NumTypes)] = { UINT16_MAX, UINT16_MAX, UINT16_MAX };
        u32 globalExecutionIndex = 0;

        for (DependencyGroup& depGroup : m_DependencyGroups)
        {
            u32 dependencyGroupExecutionIndex = 0;

            // Stores all the queues in which a resource is read
            Map<ResourceID, Set<CommandQueueType>> resourcesReadByQueues;

            for (RenderPassID passID : depGroup.Passes)
            {
                u32 queueType = u32(m_Passes[passID]->m_QueueType);

                m_OrderedPasses[globalExecutionIndex] = passID;

                m_Passes[passID]->m_GlobalExecutionIndex = globalExecutionIndex++;
                m_Passes[passID]->m_DependencyGroupExecutionIndex = dependencyGroupExecutionIndex++;
                m_Passes[passID]->m_QueueExecutionIndex = passCountPerQueue[queueType]++;

                if (prevPassPerQueue[queueType] != UINT16_MAX)
                    m_Passes[passID]->m_SyncPasses.insert(prevPassPerQueue[queueType]);

                prevPassPerQueue[queueType] = passID;

                // Track all resources read by the current pass
                for (const auto& input : m_ResourceSchedulers[Renderer::GetCurrentFrameIndex()].GetPassInputs(passID))
                {
                    resourcesReadByQueues[input->GetResourceID()].insert(m_Passes[passID]->m_QueueType);
                }
            }

            // Store all resources read by multiple queues
            for (auto& [resourceID, readingQueues] : resourcesReadByQueues)
            {
                if (readingQueues.size() > 1)
                {
                    for (CommandQueueType queueType : readingQueues)
                    {
                        depGroup.ResourcesReadByMultipleQueues.insert(resourceID);
                        depGroup.QueuesInvoledInMultiQueueReads.insert(queueType);
                    }
                }
            }
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderGraph::BuildSynchronizations()
    {
        for (DependencyGroup& depGroup : m_DependencyGroups)
        {
            // Step 1: Find closest passes to sync with for each queue
            for (RenderPassID passID : depGroup.Passes)
            {
                RenderPass* currentPass = m_Passes[passID];
                RenderPassID closestPassIDPerQueue[u32(CommandQueueType::NumTypes)] = { UINT16_MAX, UINT16_MAX, UINT16_MAX };

                for (RenderPassID syncPassID : currentPass->m_SyncPasses)
                {
                    RenderPassID closestPassID = closestPassIDPerQueue[u32(m_Passes[syncPassID]->m_QueueType)];

                    if (closestPassID == UINT16_MAX || m_Passes[closestPassID]->m_QueueExecutionIndex < m_Passes[syncPassID]->m_QueueExecutionIndex)
                    {
                        closestPassIDPerQueue[u32(m_Passes[syncPassID]->m_QueueType)] = syncPassID;
                    }
                }

                // Clear the current sync passes since there might be redundant syncs
                currentPass->m_SyncPasses.clear();

                for (u32 queueIdx = 0; queueIdx < u32(CommandQueueType::NumTypes); queueIdx++)
                {
                    RenderPassID closestPassID = closestPassIDPerQueue[queueIdx];

                    if (closestPassID == UINT16_MAX)
                    {
                        // If we don't have a node to sync with for that queue take the sync indices of the previous node executes on the same queue as the currrent node
                        closestPassID = closestPassIDPerQueue[u32(currentPass->m_QueueType)];

                        if (closestPassID != UINT16_MAX)
                            currentPass->m_SyncIndicesPerQueue[queueIdx] = m_Passes[closestPassID]->m_SyncIndicesPerQueue[queueIdx];
                    }
                    else
                    {
                        if (currentPass->m_QueueType != m_Passes[closestPassID]->m_QueueType)
                            currentPass->m_SyncIndicesPerQueue[queueIdx] = m_Passes[closestPassID]->m_QueueExecutionIndex;

                        currentPass->m_SyncPasses.insert(closestPassID);
                    }
                }

                // For its own queue, use the node's own queue execution index
                currentPass->m_SyncIndicesPerQueue[u32(currentPass->m_QueueType)] = currentPass->m_QueueExecutionIndex;
            }

            // Step 2: Remove redundant synchronizations
            for (RenderPassID passID : depGroup.Passes)
            {
                RenderPass* currentPass = m_Passes[passID];

                HashSet<CommandQueueType> queuesToSyncWith;
                HashSet<RenderPassID> optimalSyncPasses;

                for (RenderPassID syncPassID : currentPass->m_SyncPasses)
                {
                    queuesToSyncWith.insert(m_Passes[syncPassID]->m_QueueType);
                }

                while (!queuesToSyncWith.empty())
                {
                    u32 maxCoveredQueuesBySinglePass = 0;
                    HashMap<RenderPassID, Vector<CommandQueueType>> syncPassIDToCoveredQueues;

                    // Go through all sync passes and find the one covering the maximum number of queues
                    for (RenderPassID syncPassID : currentPass->m_SyncPasses)
                    {
                        RenderPass* syncPass = m_Passes[syncPassID];
                        Vector<CommandQueueType>& coveredQueues = syncPassIDToCoveredQueues[syncPassID];

                        // Check all queues and see how many we can cover with direct or indirect synchronizations
                        for (u32 queueIdx = 0; queueIdx < u32(CommandQueueType::NumTypes); queueIdx++)
                        {
                            if (u32(currentPass->m_QueueType) == queueIdx)
                            {
                                if (syncPass->m_SyncIndicesPerQueue[queueIdx] != UINT32_MAX && syncPass->m_SyncIndicesPerQueue[queueIdx] >= currentPass->m_SyncIndicesPerQueue[queueIdx] - 1)
                                    coveredQueues.push_back((CommandQueueType)queueIdx);
                            }
                            else
                            {
                                if (syncPass->m_SyncIndicesPerQueue[queueIdx] != UINT32_MAX && syncPass->m_SyncIndicesPerQueue[queueIdx] >= currentPass->m_SyncIndicesPerQueue[queueIdx])
                                    coveredQueues.push_back((CommandQueueType)queueIdx);
                            }
                        }

                        maxCoveredQueuesBySinglePass = std::max(maxCoveredQueuesBySinglePass, (u32)coveredQueues.size());
                    }

                    for (auto& [syncPassID, coveredQueues] : syncPassIDToCoveredQueues)
                    {
                        if (coveredQueues.size() < maxCoveredQueuesBySinglePass)
                            continue;

                        RenderPass* syncPass = m_Passes[syncPassID];

                        // We only need to sync passes on different queues, those running on the same queue are synchronized automatically
                        if (syncPass->m_QueueType != currentPass->m_QueueType)
                        {
                            optimalSyncPasses.insert(syncPassID);
                            currentPass->m_SyncIndicesPerQueue[u32(syncPass->m_QueueType)] = syncPass->m_QueueExecutionIndex;
                        }

                        // Remove covered queues from the list of queues we need to sync with
                        for (CommandQueueType queueType : coveredQueues)
                            queuesToSyncWith.erase(queueType);
                    }

                    // Remove sync passes the we already checked?
                }

                currentPass->m_SyncPasses = optimalSyncPasses;
            }
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderGraph::BuildResourceTransitions()
    {
        u32 frameIdx = Renderer::GetCurrentFrameIndex();
        ResourceScheduler& resourceScheduler = m_ResourceSchedulers[frameIdx];

        for (DependencyGroup& depGroup : m_DependencyGroups)
        {
            depGroup.TransitionBarriers.resize(depGroup.Passes.size());
            depGroup.UAVBarriers.resize(depGroup.Passes.size());

            for (RenderPassID passID : depGroup.Passes)
            {
                auto transitionResource = [&](ResourceID resourceID, bool isReadOnly)
                {
                    Resource* resource = resourceScheduler.GetResource(resourceID);

                    ResourceState newState = GetResourceStateForDependencyGroup(resourceID, depGroup.GroupIndex, isReadOnly);
                    std::optional<TransitionBarrier> transitionBarrier = resourceScheduler.TransitionResource(resource, newState);

                    bool needsRedirecting = false;

                    if (!transitionBarrier)
                    {
                        // Implicit transition
                        if (IsSet(newState & ResourceState::UnorderedAccess))
                            depGroup.UAVBarriers[m_Passes[passID]->m_DependencyGroupExecutionIndex].emplace_back(resource->GetHWResource());
                    }
                    else
                    {
                        bool isReadByMultipleQueues = isReadOnly && depGroup.ResourcesReadByMultipleQueues.count(resourceID);
                        bool isTransitionSupportedCurrentQueue = resourceScheduler.IsTransitionSupportedOnQueue((*transitionBarrier).GetBeforeState(), (*transitionBarrier).GetAfterState(),
                            m_Passes[passID]->m_QueueType);

                        needsRedirecting = !isTransitionSupportedCurrentQueue || isReadByMultipleQueues;

                        if (!needsRedirecting)
                            depGroup.TransitionBarriers[m_Passes[passID]->m_DependencyGroupExecutionIndex].push_back(*transitionBarrier);
                    }

                    if (needsRedirecting)
                    {
                        // Add the graphics queue as well since we will be redirecting the transitions on it
                        depGroup.QueuesInvoledInMultiQueueReads.insert(CommandQueueType::Graphics);
                        depGroup.QueuesInvoledInMultiQueueReads.insert(m_Passes[passID]->m_QueueType);
                        depGroup.RedirectedTransitionBarriers.push_back(*transitionBarrier);
                    }
                };

                for (const auto& input : resourceScheduler.GetPassInputs(passID))
                {
                    transitionResource(input->GetResourceID(), true);
                }

                for (const auto& output : resourceScheduler.GetPassOutputs(passID))
                {
                    transitionResource(output->GetResourceID(), false);
                }
            }
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderGraph::BuildRenderPassEvents()
    {
        m_RenderPassEvents.resize(m_Passes.size());

        for (RenderPassID passID : m_OrderedPasses)
        {
            const char* passName = m_Passes[passID]->m_Name.c_str();
            CommandQueueType passQueueType = m_Passes[passID]->m_QueueType;

            CommandQueue* cmdQueue = Device::Get().GetCommandQueue(passQueueType);

            RenderPassEvent passEvent;
            passEvent.PassID = passID;
            passEvent.PrePassCmdBuffer = cmdQueue->GetCommandBuffer();
            passEvent.PassCmdBuffer = cmdQueue->GetCommandBuffer();
            passEvent.PostPassCmdBuffer = cmdQueue->GetCommandBuffer();

            u32 queueIdx = u32(passQueueType);
            if (m_Passes[passID]->m_SignalRequired)
            {
                passEvent.SignalFence = m_QueueFences[queueIdx];
            }

            for (RenderPassID syncPassID : m_Passes[passID]->m_SyncPasses)
            {
                u32 syncPassQueueIdx = u32(m_Passes[syncPassID]->m_QueueType);
                passEvent.WaitFences.push_back(m_QueueFences[syncPassQueueIdx]);
            }

            m_RenderGraphEvents[queueIdx].emplace_back(passEvent);
            m_RenderPassEvents[passID] = std::prev(m_RenderGraphEvents[queueIdx].end());
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderGraph::BuildRedirectedTransitionsEvents()
    {
        HashMap<const HWResource*, std::variant<RenderPassID, u32>> resourcesLastUsage;
        for (const DependencyGroup& depGroup : m_DependencyGroups)
        {
            // Find the index of the first dependency group after which we can insert our redirected transitions for the current dependency group
            u32 redirectedTransitionsDepGroupIdx = UINT32_MAX;

            for (RenderPassID passID : depGroup.Passes)
            {
                u32 passDepGroupExecIdx = m_Passes[passID]->m_DependencyGroupExecutionIndex;
                for (const TransitionBarrier& transitionBarrier : depGroup.TransitionBarriers[passDepGroupExecIdx])
                {
                    resourcesLastUsage[transitionBarrier.GetResource()] = passID;
                }
            }

            if (depGroup.RedirectedTransitionBarriers.empty())
                continue;

            for (const TransitionBarrier& transitionBarrier : depGroup.RedirectedTransitionBarriers)
            {
                auto lastUsageInfoIt = resourcesLastUsage.find(transitionBarrier.GetResource());
                if (lastUsageInfoIt != resourcesLastUsage.end())
                {
                    u32 lastUsageDepGroupIdx = UINT32_MAX;
                    if (RenderPassID* passIDPtr = std::get_if<RenderPassID>(&lastUsageInfoIt->second))
                    {
                        lastUsageDepGroupIdx = m_Passes[*passIDPtr]->m_DependencyGroupIndex;
                    }
                    else if (u32* depGroupIdxPtr = std::get_if<u32>(&lastUsageInfoIt->second))
                    {
                        lastUsageDepGroupIdx = *depGroupIdxPtr;
                    }

                    ATOM_ENGINE_ASSERT(lastUsageDepGroupIdx != UINT32_MAX);

                    if (lastUsageDepGroupIdx != depGroup.GroupIndex && (redirectedTransitionsDepGroupIdx == UINT32_MAX || lastUsageDepGroupIdx > redirectedTransitionsDepGroupIdx))
                    {
                        redirectedTransitionsDepGroupIdx = lastUsageDepGroupIdx;
                    }
                }

                resourcesLastUsage[transitionBarrier.GetResource()] = depGroup.GroupIndex;
            }

            CommandQueueType mostCompetentQueue = CommandQueueType::Graphics;

            // Create redirected transitions event
            RedirectedTransitionsEvent transitionEvent;
            transitionEvent.DepGroupIndex = depGroup.GroupIndex;
            transitionEvent.CmdBuffer = Device::Get().GetCommandQueue(mostCompetentQueue)->GetCommandBuffer();
            transitionEvent.SignalFence = m_QueueFences[u32(mostCompetentQueue)];

            // By default we insert redirected transitions to the beginning of the event list before all passes, unless we have specific dependency group we need to wait for
            RenderGraphEventListIt insertIt = m_RenderGraphEvents[u32(mostCompetentQueue)].begin();
            if (redirectedTransitionsDepGroupIdx != UINT32_MAX)
            {
                // We have a dep group after which we have to insert the redirected transitions for the current one
                for (CommandQueueType queueToWait : depGroup.QueuesInvoledInMultiQueueReads)
                {
                    // Sync the redirected transitions with the last pass on each queue involved in transition redirecting

                    if (queueToWait == mostCompetentQueue)
                    {
                        // Will be synced automatically
                        continue;
                    }

                    // We might not have passes executing on the same queue in the dependency group after which we insert the redirected transitions so we
                    // have to find the first dependency group that contains passes on the same queue
                    s32 currentDepGroupIdx = redirectedTransitionsDepGroupIdx;

                    while (currentDepGroupIdx >= 0 && m_DependencyGroups[currentDepGroupIdx].PassesPerQueue[u32(queueToWait)].empty())
                        currentDepGroupIdx--;

                    if (currentDepGroupIdx < 0)
                    {
                        // No higher dependency group contains passes to sync with
                        continue;
                    }

                    // Get the last pass on the queue and sync the redirected transitions with it
                    RenderPassID syncPassID = m_DependencyGroups[currentDepGroupIdx].PassesPerQueue[u32(queueToWait)].back();
                    RenderPassEvent& passEvent = std::get<RenderPassEvent>(*m_RenderPassEvents[syncPassID]);

                    // Create a signal fence for the pass event if it doesn't have one
                    if (!passEvent.SignalFence)
                    {
                        passEvent.SignalFence = m_QueueFences[u32(queueToWait)];
                    }

                    // Make the redirected transitions event to wait for the pass event to signal
                    transitionEvent.WaitFences.push_back(passEvent.SignalFence);
                }

                // Insert the redirected transitions event after the last pass on the most competent queue
                RenderPassID passID = m_DependencyGroups[redirectedTransitionsDepGroupIdx].PassesPerQueue[u32(mostCompetentQueue)].back();
                insertIt = m_RenderPassEvents[passID];
                ++insertIt;
            }

            // Make the first passes on each queue involved in redirected transitions of the current dependency group wait for the redirected transitions event
            for (CommandQueueType waitingQueue : depGroup.QueuesInvoledInMultiQueueReads)
            {
                if (waitingQueue == mostCompetentQueue)
                {
                    // Will be synced automatically
                    continue;
                }

                RenderPassID waitingPassID = depGroup.PassesPerQueue[u32(waitingQueue)].front();
                RenderPassEvent& waitingPassEvent = std::get<RenderPassEvent>(*m_RenderPassEvents[waitingPassID]);
                waitingPassEvent.WaitFences.push_back(transitionEvent.SignalFence);
            }

            RenderGraphEventListIt transitionEventIt = m_RenderGraphEvents[u32(mostCompetentQueue)].emplace(insertIt, transitionEvent);
            m_RedirectedTransitionsEvents.push_back(transitionEventIt);
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderGraph::RecordRedirectedTransitions()
    {
        for (RenderGraphEventListIt& eventIt : m_RedirectedTransitionsEvents)
        {
            RedirectedTransitionsEvent& event = std::get<RedirectedTransitionsEvent>(*eventIt);

            Vector<ResourceBarrier*> redirectedBarriers;
            redirectedBarriers.reserve(m_DependencyGroups[event.DepGroupIndex].RedirectedTransitionBarriers.size());

            for (TransitionBarrier& barrier : m_DependencyGroups[event.DepGroupIndex].RedirectedTransitionBarriers)
                redirectedBarriers.push_back(&barrier);

            event.CmdBuffer->Begin();
            PIXBeginEvent(event.CmdBuffer->GetCommandList().Get(), 0, fmt::format("RedirectedTransitions_DepGroup{}", event.DepGroupIndex).c_str());
            event.CmdBuffer->ApplyBarriers(redirectedBarriers);
            PIXEndEvent(event.CmdBuffer->GetCommandList().Get());
            event.CmdBuffer->End();
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderGraph::RecordRenderPasses()
    {
        GPUDescriptorHeap* resourceHeap = Device::Get().GetGPUDescriptorHeap(DescriptorHeapType::ShaderResource);
        GPUDescriptorHeap* samplerHeap = Device::Get().GetGPUDescriptorHeap(DescriptorHeapType::Sampler);

        for (RenderPassID passID : m_OrderedPasses)
        {
            RenderPassEvent& event = std::get<RenderPassEvent>(*m_RenderPassEvents[passID]);

            RenderPass* pass = m_Passes[event.PassID];
            DependencyGroup& depGroup = m_DependencyGroups[pass->m_DependencyGroupIndex];
            u32 passIdxInGroup = pass->m_DependencyGroupExecutionIndex;

            // Record transitions
            Vector<ResourceBarrier*> barriers;
            barriers.reserve(depGroup.TransitionBarriers[passIdxInGroup].size() + depGroup.UAVBarriers[passIdxInGroup].size());

            for (TransitionBarrier& barrier : depGroup.TransitionBarriers[passIdxInGroup])
                barriers.push_back(&barrier);

            for (UAVBarrier& barrier : depGroup.UAVBarriers[passIdxInGroup])
                barriers.push_back(&barrier);

            event.PrePassCmdBuffer->Begin();

            if (!barriers.empty())
            {
                PIXBeginEvent(event.PrePassCmdBuffer->GetCommandList().Get(), 0, fmt::format("{}_Barriers", pass->m_Name.c_str()).c_str());
                event.PrePassCmdBuffer->ApplyBarriers(barriers);
                PIXEndEvent(event.PrePassCmdBuffer->GetCommandList().Get());
            }

            event.PrePassCmdBuffer->End();

            // Record render commands
            event.PassCmdBuffer->Begin();
            PIXBeginEvent(event.PassCmdBuffer->GetCommandList().Get(), 0, pass->m_Name.c_str());
            event.PassCmdBuffer->SetDescriptorHeaps(resourceHeap, samplerHeap);

            RenderPassContext passContext(pass->m_ID, event.PassCmdBuffer, m_ResourceSchedulers[Renderer::GetCurrentFrameIndex()], m_SceneRenderer.GetSceneFrameData());
            pass->Execute(passContext);

            PIXEndEvent(event.PassCmdBuffer->GetCommandList().Get());
            event.PassCmdBuffer->End();
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderGraph::ExecuteCommandLists()
    {
        // Initialize fence values for each render graph event
        for (u32 queueIdx = 0; queueIdx < (u32)CommandQueueType::NumTypes; queueIdx++)
        {
            for (RenderGraphEvent& event : m_RenderGraphEvents[queueIdx])
            {
                if (RedirectedTransitionsEvent* transitionEventPtr = std::get_if<RedirectedTransitionsEvent>(&event))
                {
                    transitionEventPtr->SignalFenceValue = m_QueueFences[queueIdx]->IncrementTargetValue();
                }
                else if (RenderPassEvent* passEventPtr = std::get_if<RenderPassEvent>(&event))
                {
                    if (passEventPtr->SignalFence)
                        passEventPtr->SignalFenceValue = m_QueueFences[queueIdx]->IncrementTargetValue();
                }
            }
        }

        // Execute
        Vector<Ref<CommandBuffer>> cmdBufferBatches[u32(CommandQueueType::NumTypes)];

        for (u32 queueIdx = 0; queueIdx < (u32)CommandQueueType::NumTypes; queueIdx++)
        {
            CommandQueue* cmdQueue = Device::Get().GetCommandQueue(CommandQueueType(queueIdx));

            for (RenderGraphEvent& event : m_RenderGraphEvents[queueIdx])
            {
                if (RedirectedTransitionsEvent* transitionEventPtr = std::get_if<RedirectedTransitionsEvent>(&event))
                {
                    if (!transitionEventPtr->WaitFences.empty())
                    {
                        // If we have fences to wait for, execute the current cmd buffer batch and wait
                        cmdQueue->ExecuteCommandLists(cmdBufferBatches[queueIdx]);
                        cmdBufferBatches[queueIdx].clear();

                        for (u32 i = 0; i < transitionEventPtr->WaitFences.size(); i++)
                            cmdQueue->WaitFence(transitionEventPtr->WaitFences[i], transitionEventPtr->WaitFenceValues[i]);
                    }

                    // Execute redirected transitions and signal fence
                    cmdQueue->ExecuteCommandList(transitionEventPtr->CmdBuffer);
                    cmdQueue->SignalFence(transitionEventPtr->SignalFence, transitionEventPtr->SignalFenceValue);
                }
                else if (RenderPassEvent* passEventPtr = std::get_if<RenderPassEvent>(&event))
                {
                    if (!passEventPtr->WaitFences.empty())
                    {
                        // If we have fences to wait for, flush the current cmd buffer batch and wait
                        cmdQueue->ExecuteCommandLists(cmdBufferBatches[queueIdx]);
                        cmdBufferBatches[queueIdx].clear();

                        for (u32 i = 0; i < passEventPtr->WaitFences.size(); i++)
                            cmdQueue->WaitFence(passEventPtr->WaitFences[i], passEventPtr->WaitFenceValues[i]);
                    }

                    if (passEventPtr->PrePassCmdBuffer)
                        cmdBufferBatches[queueIdx].push_back(passEventPtr->PrePassCmdBuffer);

                    if (passEventPtr->PassCmdBuffer)
                        cmdBufferBatches[queueIdx].push_back(passEventPtr->PassCmdBuffer);

                    //if (passEventPtr->PostPassCmdBuffer)
                    //    cmdBufferBatches[queueIdx].push_back(passEventPtr->PostPassCmdBuffer);

                    if (passEventPtr->SignalFence)
                    {
                        // If we have fences to signal, flush the current cmd buffer batch and signal
                        cmdQueue->ExecuteCommandLists(cmdBufferBatches[queueIdx]);
                        cmdQueue->SignalFence(passEventPtr->SignalFence, passEventPtr->SignalFenceValue);

                        cmdBufferBatches[queueIdx].clear();
                    }
                }
            }
        }

        // Flush any remaining cmd buffers
        for (u32 queueIdx = 0; queueIdx < (u32)CommandQueueType::NumTypes; queueIdx++)
        {
            CommandQueue* cmdQueue = Device::Get().GetCommandQueue(CommandQueueType(queueIdx));
            cmdQueue->ExecuteCommandLists(cmdBufferBatches[queueIdx]);
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderGraph::DFS(RenderPassID passID, Vector<RenderPassID>& sortedPasses, Vector<bool>& visited, Vector<bool>& nodesOnStack)
    {
        visited[passID] = true;
        nodesOnStack[passID] = true;

        for (RenderPassID neighbourPassID : m_AdjacencyList[passID])
        {
            ATOM_ENGINE_ASSERT(!visited[neighbourPassID] || !nodesOnStack[neighbourPassID], "Cycle detected!");

            if (!visited[neighbourPassID])
                DFS(neighbourPassID, sortedPasses, visited, nodesOnStack);
        }

        sortedPasses.push_back(passID);
        nodesOnStack[passID] = false;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    ResourceState RenderGraph::GetResourceStateForDependencyGroup(ResourceID resourceID, u32 depGroupIdx, bool isReadOnly)
    {
        u32 frameIdx = Renderer::GetCurrentFrameIndex();
        ResourceScheduler& resourceScheduler = m_ResourceSchedulers[frameIdx];

        ResourceState state = ResourceState::Common;
        for (RenderPassID passID : m_DependencyGroups[depGroupIdx].Passes)
        {
            if (isReadOnly)
            {
                for (auto& input : resourceScheduler.GetPassInputs(passID))
                {
                    if (input->GetResourceID() == resourceID)
                    {
                        if (input->As<TextureSRV>())
                        {
                            state |= (m_Passes[passID]->m_QueueType == CommandQueueType::Compute ? ResourceState::NonPixelShaderRead : ResourceState::PixelShaderRead);
                        }
                        else if (input->As<SurfaceDSV_RO>())
                        {
                            state |= ResourceState::DepthRead;
                        }
                    }
                }
            }
            else
            {
                for (auto& output : resourceScheduler.GetPassOutputs(passID))
                {
                    if (output->GetResourceID() == resourceID)
                    {
                        if (output->As<TextureUAV>())
                        {
                            state |= ResourceState::UnorderedAccess;
                        }
                        else if (output->As<SurfaceRTV>())
                        {
                            state |= ResourceState::RenderTarget;
                        }
                        else if (output->As<SurfaceDSV_RW>())
                        {
                            state |= ResourceState::DepthWrite;
                        }
                    }
                }
            }
        }

        return state;
    }
}
