#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Renderer/CommandQueue.h"

namespace Atom
{
    class RenderPassBuilder;
    class RenderPassContext;

    using RenderPassID = u16;

    class RenderPass
    {
        friend class RenderGraph;
    public:
        using BuildCallback = std::function<void(RenderPassBuilder&)>;
        using ExecuteCallback = std::function<void(RenderPassContext&)>;
    public:
        RenderPass(RenderPassID id, const String& name, CommandQueueType queueType, const BuildCallback& buildFn, const ExecuteCallback& executeFn);
        ~RenderPass();

        void Build(RenderPassBuilder& builder);
        void Execute();

        inline RenderPassID GetID() const { return m_ID; }
        inline const String& GetName() const { return m_Name; }
        inline CommandQueueType GetQueueType() const { return m_QueueType; }
    private:
        RenderPassID             m_ID;
        String                   m_Name;
        CommandQueueType         m_QueueType;
        BuildCallback            m_BuildFn;
        ExecuteCallback          m_ExecuteFn;

        // Execution info
        RenderPassContext*       m_Context;
        HashSet<RenderPassID>    m_SyncPasses;
        u32                      m_SyncIndicesPerQueue[u32(CommandQueueType::NumTypes)] = { UINT32_MAX, UINT32_MAX, UINT32_MAX };
        u32                      m_DependencyGroupIndex = UINT32_MAX;
        u32                      m_GlobalExecutionIndex = UINT32_MAX;
        u32                      m_DependencyGroupExecutionIndex = UINT32_MAX;
        u32                      m_QueueExecutionIndex = UINT32_MAX;
    };
}