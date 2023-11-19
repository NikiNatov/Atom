#include "atompch.h"
#include "RenderPassContext.h"

#include "Atom/Renderer/RenderGraph/ResourceScheduler.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    RenderPassContext::RenderPassContext(RenderPassID passID, Ref<CommandBuffer> cmdBuffer, const ResourceScheduler& resourceScheduler)
        : m_PassID(passID), m_CommandBuffer(cmdBuffer), m_ResourceScheduler(resourceScheduler)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    ResourceView<TextureUAV>* RenderPassContext::GetUA(const ResourceID_UA& id) const
    {
        IResourceView* view = FindOutput(id);
        ATOM_ENGINE_ASSERT(view, "No UAV found for resource");
        ATOM_ENGINE_ASSERT(view->As<TextureUAV>(), "View type mismatch");
        return dynamic_cast<ResourceView<TextureUAV>*>(view);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    ResourceView<TextureSRV>* RenderPassContext::GetSR(const ResourceID_UA& id) const
    {
        IResourceView* view = FindInput(id);
        ATOM_ENGINE_ASSERT(view, "No SRV found for resource");
        ATOM_ENGINE_ASSERT(view->As<TextureSRV>(), "View type mismatch");
        return dynamic_cast<ResourceView<TextureSRV>*>(view);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    ResourceView<SurfaceRTV>* RenderPassContext::GetRT(const ResourceID_RT& id) const
    {
        IResourceView* view = FindOutput(id);
        ATOM_ENGINE_ASSERT(view, "No RTV found for resource");
        ATOM_ENGINE_ASSERT(view->As<SurfaceRTV>(), "View type mismatch");
        return dynamic_cast<ResourceView<SurfaceRTV>*>(view);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    ResourceView<SurfaceSRV>* RenderPassContext::GetSR(const ResourceID_RT& id) const
    {
        IResourceView* view = FindInput(id);
        ATOM_ENGINE_ASSERT(view, "No SRV found for resource");
        ATOM_ENGINE_ASSERT(view->As<SurfaceSRV>(), "View type mismatch");
        return dynamic_cast<ResourceView<SurfaceSRV>*>(view);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    ResourceView<SurfaceDSV_RW>* RenderPassContext::GetDS_RW(const ResourceID_DS& id) const
    {
        IResourceView* view = FindOutput(id);
        ATOM_ENGINE_ASSERT(view, "No DSV found for resource");
        ATOM_ENGINE_ASSERT(view->As<SurfaceDSV_RW>(), "View type mismatch");
        return dynamic_cast<ResourceView<SurfaceDSV_RW>*>(view);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    ResourceView<SurfaceDSV_RO>* RenderPassContext::GetDS_RO(const ResourceID_DS& id) const
    {
        IResourceView* view = FindOutput(id);
        ATOM_ENGINE_ASSERT(view, "No DSV found for resource");
        ATOM_ENGINE_ASSERT(view->As<SurfaceDSV_RO>(), "View type mismatch");
        return dynamic_cast<ResourceView<SurfaceDSV_RO>*>(view);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    ResourceView<SurfaceSRV>* RenderPassContext::GetSR(const ResourceID_DS& id) const
    {
        IResourceView* view = FindInput(id);
        ATOM_ENGINE_ASSERT(view, "No SRV found for resource");
        ATOM_ENGINE_ASSERT(view->As<SurfaceSRV>(), "View type mismatch");
        return dynamic_cast<ResourceView<SurfaceSRV>*>(view);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    IResourceView* RenderPassContext::FindInput(const ResourceID& id) const
    {
        for (const auto& view : m_ResourceScheduler.GetPassInputs(m_PassID))
            if (view->GetResourceID() == id)
                return view;

        return nullptr;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    IResourceView* RenderPassContext::FindOutput(const ResourceID& id) const
    {
        for (const auto& view : m_ResourceScheduler.GetPassOutputs(m_PassID))
            if (view->GetResourceID() == id)
                return view;

        return nullptr;
    }
}
