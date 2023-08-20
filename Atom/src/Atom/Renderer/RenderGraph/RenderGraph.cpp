#include "atompch.h"
#include "RenderGraph.h"

#include "Atom/Renderer/RenderGraph/RenderPassBuilder.h"
#include "Atom/Renderer/RenderGraph/RenderPassContext.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    RenderGraph::RenderGraph()
    {
        m_Resources.resize(ResourceIDRegistry::GetResourceCount(), nullptr);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderGraph::Build()
    {
        BuildAdjacencyLists();
        BuildExecutionOrder();
        BuildDependencyGroups();
        BuildResourceLifetimes();
        BuildSynchronizations();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderGraph::Execute()
    {
        
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderGraph::Reset()
    {
        m_Passes.clear();
        m_SortedPasses.clear();
        m_FinalExecutionOrderPasses.clear();
        m_AdjacencyList.clear();
        m_DependencyGroups.clear();

        for (auto& resource : m_Resources)
            resource = nullptr;

        m_ResourceLifetimes.clear();
    }


    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderGraph::BuildAdjacencyLists()
    {
        // Build pass resource contexts
        for (auto& pass : m_Passes)
        {
            RenderPassBuilder passBuilder(*this, pass.m_ID);
            pass.Build(passBuilder);
        }

        // Create adjacency lists
        m_AdjacencyList.resize(m_Passes.size());
        for (auto& pass : m_Passes)
        {
            for (auto& otherPass : m_Passes)
            {
                if (otherPass.m_ID == pass.m_ID)
                    continue;

                for (auto& input : otherPass.m_Context->GetInputs())
                {
                    if (m_Resources[input->GetResourceID().GetIndex()]->GetProducerPassID() == pass.m_ID)
                    {
                        m_AdjacencyList[pass.m_ID].push_back(otherPass.m_ID);

                        if (pass.m_QueueType != otherPass.m_QueueType)
                        {
                            otherPass.m_SyncPasses.insert(pass.m_ID);
                        }

                        break;
                    }
                }
            }
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderGraph::BuildExecutionOrder()
    {
        Vector<bool> visited(m_Passes.size(), false);
        Vector<bool> nodesOnStack(m_Passes.size(), false);

        for (RenderPassID passID = 0; passID < m_Passes.size(); passID++)
            if (!visited[passID])
                DFS(passID, visited, nodesOnStack);

        std::reverse(m_SortedPasses.begin(), m_SortedPasses.end());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderGraph::BuildDependencyGroups()
    {
        // Get max distances for each node from the root
        Vector<u32> maxDistances(m_Passes.size(), 0);

        u32 numDependencyGroups = 1;
        for (RenderPassID passID : m_SortedPasses)
        {
            for (RenderPassID neighbourPassID : m_AdjacencyList[passID])
            {
                maxDistances[neighbourPassID] = std::max(maxDistances[neighbourPassID], maxDistances[passID] + 1);
                numDependencyGroups = std::max(numDependencyGroups, maxDistances[neighbourPassID] + 1);
            }
        }

        // Add every pass node to its corresponding dependency group
        m_DependencyGroups.resize(numDependencyGroups);
        for (RenderPass& pass : m_Passes)
        {
            pass.m_DependencyGroupIndex = maxDistances[pass.m_ID];
            m_DependencyGroups[pass.m_DependencyGroupIndex].GroupIndex = pass.m_DependencyGroupIndex;
            m_DependencyGroups[pass.m_DependencyGroupIndex].Passes.push_back(pass.m_ID);
            m_DependencyGroups[pass.m_DependencyGroupIndex].PassesPerQueue[u32(pass.m_QueueType)].push_back(pass.m_ID);
        }

        // Set execution indices and build final execution order of the passes
        m_FinalExecutionOrderPasses.resize(m_SortedPasses.size());

        u32 passCountPerQueue[u32(CommandQueueType::NumTypes)] = { 0, 0, 0 };
        RenderPassID prevPassPerQueue[u32(CommandQueueType::NumTypes)] = { UINT16_MAX, UINT16_MAX, UINT16_MAX };
        u32 globalExecutionIndex = 0;

        for (DependencyGroup& depGroup : m_DependencyGroups)
        {
            u32 dependencyGroupExecutionIndex = 0;

            for (RenderPassID passID : depGroup.Passes)
            {
                u32 queueType = u32(m_Passes[passID].m_QueueType);

                m_FinalExecutionOrderPasses[globalExecutionIndex] = passID;

                m_Passes[passID].m_GlobalExecutionIndex = globalExecutionIndex++;
                m_Passes[passID].m_DependencyGroupExecutionIndex = dependencyGroupExecutionIndex++;
                m_Passes[passID].m_QueueExecutionIndex = passCountPerQueue[queueType]++;

                if (prevPassPerQueue[queueType] != UINT16_MAX)
                    m_Passes[passID].m_SyncPasses.insert(prevPassPerQueue[queueType]);

                prevPassPerQueue[queueType] = passID;
            }
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderGraph::BuildResourceLifetimes()
    {
        m_ResourceLifetimes.resize(m_Resources.size());

        for (RenderPassID passID : m_FinalExecutionOrderPasses)
        {
            for (auto& input : m_Passes[passID].m_Context->GetInputs())
            {
                ResourceLifetime& lifetime = m_ResourceLifetimes[input->GetResourceID().GetIndex()];
                if (lifetime.FirstPass == UINT16_MAX)
                {
                    // Create resource lifetime
                    lifetime.FirstPass = m_Passes[passID].m_GlobalExecutionIndex;
                    lifetime.EndPass = m_Passes[passID].m_GlobalExecutionIndex;
                }
                else
                {
                    // Update end pass
                    lifetime.EndPass = m_Passes[passID].m_GlobalExecutionIndex;
                }
            }

            for (auto& output : m_Passes[passID].m_Context->GetOutputs())
            {
                ResourceLifetime& lifetime = m_ResourceLifetimes[output->GetResourceID().GetIndex()];
                if (lifetime.FirstPass == UINT16_MAX)
                {
                    // Create resource lifetime
                    lifetime.FirstPass = m_Passes[passID].m_GlobalExecutionIndex;
                    lifetime.EndPass = m_Passes[passID].m_GlobalExecutionIndex;
                }
                else
                {
                    // Update end pass
                    lifetime.EndPass = m_Passes[passID].m_GlobalExecutionIndex;
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
                RenderPass& currentPass = m_Passes[passID];
                RenderPassID closestPassIDPerQueue[u32(CommandQueueType::NumTypes)] = { UINT16_MAX, UINT16_MAX, UINT16_MAX };

                for (RenderPassID syncPassID : currentPass.m_SyncPasses)
                {
                    RenderPassID closestPassID = closestPassIDPerQueue[u32(m_Passes[syncPassID].m_QueueType)];

                    if (closestPassID == UINT16_MAX || m_Passes[closestPassID].m_QueueExecutionIndex < m_Passes[syncPassID].m_QueueExecutionIndex)
                    {
                        closestPassIDPerQueue[u32(m_Passes[syncPassID].m_QueueType)] = syncPassID;
                    }
                }

                // Clear the current sync passes since there might be redundant syncs
                currentPass.m_SyncPasses.clear();

                for (u32 queueIdx = 0; queueIdx < u32(CommandQueueType::NumTypes); queueIdx++)
                {
                    RenderPassID closestPassID = closestPassIDPerQueue[queueIdx];

                    if (closestPassID == UINT16_MAX)
                    {
                        // If we don't have a node to sync with for that queue take the sync indices of the previous node executes on the same queue as the currrent node
                        closestPassID = closestPassIDPerQueue[u32(currentPass.m_QueueType)];

                        if (closestPassID != UINT16_MAX)
                            currentPass.m_SyncIndicesPerQueue[queueIdx] = m_Passes[closestPassID].m_SyncIndicesPerQueue[queueIdx];
                    }
                    else
                    {
                        if (currentPass.m_QueueType != m_Passes[closestPassID].m_QueueType)
                            currentPass.m_SyncIndicesPerQueue[queueIdx] = m_Passes[closestPassID].m_QueueExecutionIndex;

                        currentPass.m_SyncPasses.insert(closestPassID);
                    }
                }

                // For its own queue, use the node's own queue execution index
                currentPass.m_SyncIndicesPerQueue[u32(currentPass.m_QueueType)] = currentPass.m_QueueExecutionIndex;
            }

            // Step 2: Remove redundant synchronizations
            for (RenderPassID passID : depGroup.Passes)
            {
                RenderPass& currentPass = m_Passes[passID];

                HashSet<CommandQueueType> queuesToSyncWith;
                HashSet<RenderPassID> optimalSyncPasses;

                for (RenderPassID syncPassID : currentPass.m_SyncPasses)
                {
                    queuesToSyncWith.insert(m_Passes[syncPassID].m_QueueType);
                }

                while (!queuesToSyncWith.empty())
                {
                    u32 maxCoveredQueuesBySinglePass = 0;
                    HashMap<RenderPassID, Vector<CommandQueueType>> syncPassIDToCoveredQueues;

                    // Go through all sync passes and find the one covering the maximum number of queues
                    for (RenderPassID syncPassID : currentPass.m_SyncPasses)
                    {
                        RenderPass& syncPass = m_Passes[syncPassID];
                        Vector<CommandQueueType>& coveredQueues = syncPassIDToCoveredQueues[syncPassID];

                        // Check all queues and see how many we can cover with direct or indirect synchronizations
                        for (u32 queueIdx = 0; queueIdx < u32(CommandQueueType::NumTypes); queueIdx++)
                        {
                            if (u32(currentPass.m_QueueType) == queueIdx)
                            {
                                if (syncPass.m_SyncIndicesPerQueue[queueIdx] != UINT32_MAX && syncPass.m_SyncIndicesPerQueue[queueIdx] >= currentPass.m_SyncIndicesPerQueue[queueIdx] - 1)
                                    coveredQueues.push_back((CommandQueueType)queueIdx);
                            }
                            else
                            {
                                if (syncPass.m_SyncIndicesPerQueue[queueIdx] != UINT32_MAX && syncPass.m_SyncIndicesPerQueue[queueIdx] >= currentPass.m_SyncIndicesPerQueue[queueIdx])
                                    coveredQueues.push_back((CommandQueueType)queueIdx);
                            }
                        }

                        maxCoveredQueuesBySinglePass = std::max(maxCoveredQueuesBySinglePass, (u32)coveredQueues.size());
                    }

                    for (auto& [syncPassID, coveredQueues] : syncPassIDToCoveredQueues)
                    {
                        if (coveredQueues.size() < maxCoveredQueuesBySinglePass)
                            continue;

                        RenderPass& syncPass = m_Passes[syncPassID];

                        // We only need to sync passes on different queues, those running on the same queue are synchronized automatically
                        if (syncPass.m_QueueType != currentPass.m_QueueType)
                        {
                            optimalSyncPasses.insert(syncPassID);
                            currentPass.m_SyncIndicesPerQueue[u32(syncPass.m_QueueType)] = syncPass.m_QueueExecutionIndex;
                        }

                        // Remove covered queues from the list of queues we need to sync with
                        for (CommandQueueType queueType : coveredQueues)
                            queuesToSyncWith.erase(queueType);
                    }

                    // Remove sync passes the we already checked?
                }

                currentPass.m_SyncPasses = optimalSyncPasses;
            }
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderGraph::DFS(RenderPassID passID, Vector<bool>& visited, Vector<bool>& nodesOnStack)
    {
        visited[passID] = true;
        nodesOnStack[passID] = true;

        for (RenderPassID neighbourPassID : m_AdjacencyList[passID])
        {
            ATOM_ENGINE_ASSERT(!visited[neighbourPassID] || !nodesOnStack[neighbourPassID], "Cycle detected!");

            if (!visited[neighbourPassID])
                DFS(neighbourPassID, visited, nodesOnStack);
        }

        m_SortedPasses.push_back(passID);
        nodesOnStack[passID] = false;
    }
    
}
