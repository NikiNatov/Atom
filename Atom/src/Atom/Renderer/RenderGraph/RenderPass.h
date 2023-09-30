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
        virtual ~RenderPass() = default;

        virtual void Build(RenderPassBuilder& builder) = 0;
        virtual void Execute(RenderPassContext& context) = 0;

        inline RenderPassID GetID() const { return m_ID; }
        inline const String& GetName() const { return m_Name; }
        inline CommandQueueType GetQueueType() const { return m_QueueType; }
    protected:
        RenderPass(RenderPassID id, const String& name, CommandQueueType queueType);
    protected:
        RenderPassID             m_ID;
        String                   m_Name;
        CommandQueueType         m_QueueType;

        // Execution info
        bool                     m_SignalRequired = false;
        HashSet<RenderPassID>    m_SyncPasses;
        u32                      m_SyncIndicesPerQueue[u32(CommandQueueType::NumTypes)] = { UINT32_MAX, UINT32_MAX, UINT32_MAX };
        u32                      m_DependencyGroupIndex = UINT32_MAX;
        u32                      m_GlobalExecutionIndex = UINT32_MAX;
        u32                      m_DependencyGroupExecutionIndex = UINT32_MAX;
        u32                      m_QueueExecutionIndex = UINT32_MAX;
    };
}