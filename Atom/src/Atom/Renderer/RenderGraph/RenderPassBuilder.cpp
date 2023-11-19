#include "atompch.h"
#include "RenderPassBuilder.h"

#include "Atom/Renderer/RenderGraph/ResourceScheduler.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    RenderPassBuilder::RenderPassBuilder(RenderPassID passID, ResourceScheduler& resourceScheduler)
        : m_ResourceScheduler(resourceScheduler), m_PassID(passID)
    {
        ATOM_ENGINE_ASSERT(m_PassID != UINT16_MAX);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderPassBuilder::NewUA(const ResourceID_UA& id, const ResourceID_UA::ResourceType::ResourceDescType& description)
    {
        m_ResourceScheduler.CreateResource<ResourceID_UA::ResourceType>(id, description)->SetProducerPassID(m_PassID);
        m_ResourceScheduler.CreateResourceView<TextureUAV>(m_PassID, id);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderPassBuilder::NewUA(const ResourceID_UA& id, ResourceID_UA::ResourceType::HWResourceType* externalResource)
    {
        m_ResourceScheduler.CreateResource<ResourceID_UA::ResourceType>(id, externalResource)->SetProducerPassID(m_PassID);
        m_ResourceScheduler.CreateResourceView<TextureUAV>(m_PassID, id);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderPassBuilder::NewRT(const ResourceID_RT& id, const ResourceID_RT::ResourceType::ResourceDescType& description)
    {
        m_ResourceScheduler.CreateResource<ResourceID_RT::ResourceType>(id, description)->SetProducerPassID(m_PassID);
        m_ResourceScheduler.CreateResourceView<SurfaceRTV>(m_PassID, id);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderPassBuilder::NewRT(const ResourceID_RT& id, ResourceID_RT::ResourceType::HWResourceType* externalResource)
    {
        m_ResourceScheduler.CreateResource<ResourceID_RT::ResourceType>(id, externalResource)->SetProducerPassID(m_PassID);
        m_ResourceScheduler.CreateResourceView<SurfaceRTV>(m_PassID, id);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderPassBuilder::NewDS(const ResourceID_DS& id, const ResourceID_DS::ResourceType::ResourceDescType& description)
    {
        m_ResourceScheduler.CreateResource<ResourceID_DS::ResourceType>(id, description)->SetProducerPassID(m_PassID);
        m_ResourceScheduler.CreateResourceView<SurfaceDSV_RW>(m_PassID, id);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderPassBuilder::NewDS(const ResourceID_DS& id, ResourceID_DS::ResourceType::HWResourceType* externalResource)
    {
        m_ResourceScheduler.CreateResource<ResourceID_DS::ResourceType>(id, externalResource)->SetProducerPassID(m_PassID);
        m_ResourceScheduler.CreateResourceView<SurfaceDSV_RW>(m_PassID, id);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderPassBuilder::Read(const ResourceID_UA& id)
    {
        m_ResourceScheduler.CreateResourceView<TextureSRV>(m_PassID, id);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderPassBuilder::Read(const ResourceID_RT& id)
    {
        m_ResourceScheduler.CreateResourceView<SurfaceSRV>(m_PassID, id);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderPassBuilder::Read(const ResourceID_DS& id, bool isSRV)
    {
        if(isSRV)
            m_ResourceScheduler.CreateResourceView<SurfaceSRV>(m_PassID, id);
        else
            m_ResourceScheduler.CreateResourceView<SurfaceDSV_RO>(m_PassID, id);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderPassBuilder::Write(const ResourceID_UA& id)
    {
        m_ResourceScheduler.CreateResourceView<TextureUAV>(m_PassID, id);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderPassBuilder::Write(const ResourceID_RT& id)
    {
        m_ResourceScheduler.CreateResourceView<SurfaceRTV>(m_PassID, id);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderPassBuilder::Write(const ResourceID_DS& id)
    {
        m_ResourceScheduler.CreateResourceView<SurfaceDSV_RW>(m_PassID, id);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderPassBuilder::SetPipelineStateDesc(const GraphicsPipelineDescription& description)
    {
        m_ResourceScheduler.AssignPipelineToPass(m_PassID, description);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderPassBuilder::SetPipelineStateDesc(const ComputePipelineDescription& description)
    {
        m_ResourceScheduler.AssignPipelineToPass(m_PassID, description);
    }
}
