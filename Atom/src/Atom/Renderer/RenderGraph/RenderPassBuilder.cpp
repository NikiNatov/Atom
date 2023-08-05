#include "atompch.h"
#include "RenderPassBuilder.h"

#include "Atom/Renderer/RenderGraph/RenderGraph.h"
#include "Atom/Renderer/RenderGraph/ResourceView.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    RenderPassBuilder::RenderPassBuilder(RenderGraph& graph, RenderPassID passID)
        : m_Graph(graph), m_PassID(passID)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    RenderPassContext* RenderPassBuilder::Finalize()
    {
        return new RenderPassContext(m_Graph.GetRenderPass(m_PassID), std::move(m_Inputs), std::move(m_Outputs));
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderPassBuilder::NewUA(ResourceID_UA id, const TextureDescription& description)
    {
        m_Graph.CreateResource<TextureResource>(id, description)->SetProducerPassID(m_PassID);
        m_Outputs.push_back(CreateScope<ResourceView<TextureUAV>>(&m_Graph, id));
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderPassBuilder::NewRT(ResourceID_RT id, const TextureDescription& description)
    {
        m_Graph.CreateResource<RenderSurfaceResource>(id, description)->SetProducerPassID(m_PassID);
        m_Outputs.push_back(CreateScope<ResourceView<SurfaceRTV>>(&m_Graph, id));
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderPassBuilder::NewDS(ResourceID_DS id, const TextureDescription& description)
    {
        m_Graph.CreateResource<RenderSurfaceResource>(id, description)->SetProducerPassID(m_PassID);
        m_Outputs.push_back(CreateScope<ResourceView<SurfaceDSV>>(&m_Graph, id));
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderPassBuilder::Read(ResourceID_UA id)
    {
        ATOM_ENGINE_ASSERT(!HasOutput(id), "Resource already added as an output!");
        if(!HasInput(id))
            m_Inputs.push_back(CreateScope<ResourceView<TextureSRV>>(&m_Graph, id));
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderPassBuilder::Read(ResourceID_RT id)
    {
        ATOM_ENGINE_ASSERT(!HasOutput(id), "Resource already added as an output!");
        if (!HasInput(id))
            m_Inputs.push_back(CreateScope<ResourceView<TextureSRV>>(&m_Graph, id));
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderPassBuilder::Read(ResourceID_DS id)
    {
        ATOM_ENGINE_ASSERT(!HasOutput(id), "Resource already added as an output!");
        if (!HasInput(id))
            m_Inputs.push_back(CreateScope<ResourceView<TextureSRV>>(&m_Graph, id));
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderPassBuilder::Write(ResourceID_UA id)
    {
        ATOM_ENGINE_ASSERT(!HasInput(id), "Resource already added as an input!");
        if (!HasOutput(id))
            m_Inputs.push_back(CreateScope<ResourceView<TextureUAV>>(&m_Graph, id));
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderPassBuilder::Write(ResourceID_RT id)
    {
        ATOM_ENGINE_ASSERT(!HasInput(id), "Resource already added as an input!");
        if (!HasOutput(id))
            m_Inputs.push_back(CreateScope<ResourceView<SurfaceRTV>>(&m_Graph, id));
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderPassBuilder::Write(ResourceID_DS id)
    {
        ATOM_ENGINE_ASSERT(!HasInput(id), "Resource already added as an input!");
        if (!HasOutput(id))
            m_Inputs.push_back(CreateScope<ResourceView<SurfaceDSV>>(&m_Graph, id));
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool RenderPassBuilder::HasInput(ResourceID id) const
    {
        for (const auto& view : m_Inputs)
            if (view->GetResourceID() == id)
                return true;

        return false;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool RenderPassBuilder::HasOutput(ResourceID id) const
    {
        for (const auto& view : m_Outputs)
            if (view->GetResourceID() == id)
                return true;

        return false;
    }
}
