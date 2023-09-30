#include "atompch.h"
#include "RenderPass.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    RenderPass::RenderPass(RenderPassID id, const String& name, CommandQueueType queueType)
        : m_ID(id), m_Name(name), m_QueueType(queueType)
    {
    }
}
