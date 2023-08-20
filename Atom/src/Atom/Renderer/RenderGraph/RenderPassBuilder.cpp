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
        const auto& resource = m_Graph.CreateResource<TextureResource>(id, description);
        resource->SetProducerPassID(m_PassID);
        m_Outputs.push_back(CreateScope<ResourceView<TextureResource, TextureUAV>>(resource.get()));
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderPassBuilder::NewRT(ResourceID_RT id, const TextureDescription& description)
    {
        const auto& resource = m_Graph.CreateResource<RenderSurfaceResource>(id, description);
        resource->SetProducerPassID(m_PassID);
        m_Outputs.push_back(CreateScope<ResourceView<RenderSurfaceResource, SurfaceRTV>>(resource.get()));
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderPassBuilder::NewDS(ResourceID_DS id, const TextureDescription& description)
    {
        const auto& resource = m_Graph.CreateResource<RenderSurfaceResource>(id, description);
        resource->SetProducerPassID(m_PassID);
        m_Outputs.push_back(CreateScope<ResourceView<RenderSurfaceResource, SurfaceDSV>>(resource.get()));
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderPassBuilder::Read(ResourceID_UA id)
    {
        ATOM_ENGINE_ASSERT(!HasOutput(id), "Resource already added as an output!");
        if (!HasInput(id))
        {
            const auto& resource = m_Graph.GetResource<TextureResource>(id);
            m_Inputs.push_back(CreateScope<ResourceView<TextureResource, TextureSRV>>(resource.get()));
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderPassBuilder::Read(ResourceID_RT id)
    {
        ATOM_ENGINE_ASSERT(!HasOutput(id), "Resource already added as an output!");
        if (!HasInput(id))
        {
            const auto& resource = m_Graph.GetResource<RenderSurfaceResource>(id);
            m_Inputs.push_back(CreateScope<ResourceView<RenderSurfaceResource, TextureSRV>>(resource.get()));
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderPassBuilder::Read(ResourceID_DS id, bool isSRV)
    {
        ATOM_ENGINE_ASSERT(!HasOutput(id), "Resource already added as an output!");
        if (!HasInput(id))
        {
            const auto& resource = m_Graph.GetResource<RenderSurfaceResource>(id);
            if (isSRV)
                m_Inputs.push_back(CreateScope<ResourceView<RenderSurfaceResource, TextureSRV>>(resource.get()));
            else
                m_Inputs.push_back(CreateScope<ResourceView<RenderSurfaceResource, SurfaceDSV>>(resource.get()));
            
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderPassBuilder::Write(ResourceID_UA id)
    {
        ATOM_ENGINE_ASSERT(!HasInput(id), "Resource already added as an input!");
        if (!HasOutput(id))
        {
            const auto& resource = m_Graph.GetResource<TextureResource>(id);
            m_Inputs.push_back(CreateScope<ResourceView<TextureResource, TextureUAV>>(resource.get()));
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderPassBuilder::Write(ResourceID_RT id)
    {
        ATOM_ENGINE_ASSERT(!HasInput(id), "Resource already added as an input!");
        if (!HasOutput(id))
        {
            const auto& resource = m_Graph.GetResource<RenderSurfaceResource>(id);
            m_Inputs.push_back(CreateScope<ResourceView<RenderSurfaceResource, SurfaceRTV>>(resource.get()));
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderPassBuilder::Write(ResourceID_DS id)
    {
        ATOM_ENGINE_ASSERT(!HasInput(id), "Resource already added as an input!");
        if (!HasOutput(id))
        {
            const auto& resource = m_Graph.GetResource<RenderSurfaceResource>(id);
            m_Inputs.push_back(CreateScope<ResourceView<RenderSurfaceResource, SurfaceDSV>>(resource.get()));
        }
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
