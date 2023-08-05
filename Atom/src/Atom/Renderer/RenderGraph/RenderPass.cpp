#include "atompch.h"
#include "RenderPass.h"

#include "Atom/Renderer/RenderGraph/RenderPassBuilder.h"
#include "Atom/Renderer/RenderGraph/RenderPassContext.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    RenderPass::RenderPass(RenderPassID id, const String& name, CommandQueueType queueType, const BuildCallback& buildFn, const ExecuteCallback& executeFn)
        : m_ID(id), m_Name(name), m_QueueType(queueType), m_BuildFn(buildFn), m_ExecuteFn(executeFn), m_Context(nullptr)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    RenderPass::~RenderPass()
    {
        delete m_Context;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderPass::Build(RenderPassBuilder& builder)
    {
        ATOM_ENGINE_ASSERT(m_ID == builder.GetPassID());
        m_BuildFn(builder);
        m_Context = builder.Finalize();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderPass::Execute()
    {
        ATOM_ENGINE_ASSERT(m_Context, "Render pass was not build!")
        m_ExecuteFn(*m_Context);
    }
}
