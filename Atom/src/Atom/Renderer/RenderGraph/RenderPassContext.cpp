#include "atompch.h"
#include "RenderPassContext.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    RenderPassContext::RenderPassContext(const RenderPass& pass, Vector<Scope<IResourceView>>&& inputs, Vector<Scope<IResourceView>>&& outputs)
        : m_CommandBuffer(CreateRef<CommandBuffer>(pass.GetQueueType(), pass.GetName().c_str())), m_Inputs(std::move(inputs)), m_Outputs(std::move(outputs))
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const ResourceView<TextureUAV>& RenderPassContext::GetUA(ResourceID_UA id) const
    {
        const IResourceView* view = FindOutput(id);
        ATOM_ENGINE_ASSERT(view, "No UAV found for resource");
        return *(ResourceView<TextureUAV>*)view;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const ResourceView<TextureSRV>& RenderPassContext::GetSR(ResourceID_UA id) const
    {
        const IResourceView* view = FindInput(id);
        ATOM_ENGINE_ASSERT(view, "No SRV found for resource");
        return *(ResourceView<TextureSRV>*)view;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const ResourceView<SurfaceRTV>& RenderPassContext::GetRT(ResourceID_RT id) const
    {
        const IResourceView* view = FindOutput(id);
        ATOM_ENGINE_ASSERT(view, "No RTV found for resource");
        return *(ResourceView<SurfaceRTV>*)view;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const ResourceView<TextureSRV>& RenderPassContext::GetSR(ResourceID_RT id) const
    {
        const IResourceView* view = FindInput(id);
        ATOM_ENGINE_ASSERT(view, "No SRV found for resource");
        return *(ResourceView<TextureSRV>*)view;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const ResourceView<SurfaceDSV>& RenderPassContext::GetDS(ResourceID_DS id) const
    {
        const IResourceView* view = FindOutput(id);
        ATOM_ENGINE_ASSERT(view, "No DSV found for resource");
        return *(ResourceView<SurfaceDSV>*)view;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const ResourceView<TextureSRV>& RenderPassContext::GetSR(ResourceID_DS id) const
    {
        const IResourceView* view = FindInput(id);
        ATOM_ENGINE_ASSERT(view, "No SRV found for resource");
        return *(ResourceView<TextureSRV>*)view;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const IResourceView* RenderPassContext::FindInput(ResourceID id) const
    {
        for (const auto& view : m_Inputs)
            if (view->GetResourceID() == id)
                return view.get();

        return nullptr;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const IResourceView* RenderPassContext::FindOutput(ResourceID id) const
    {
        for (const auto& view : m_Outputs)
            if (view->GetResourceID() == id)
                return view.get();

        return nullptr;
    }
}
