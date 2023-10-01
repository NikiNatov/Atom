#include "atompch.h"
#include "ResourceView.h"

#include "Atom/Renderer/RenderGraph/ResourceScheduler.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    template<>
    typename TextureUAV::HWResourceType* ResourceView<TextureUAV>::GetData() const
    {
        return GetData(TextureView::AllMips, TextureView::AllSlices);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    template<>
    typename TextureUAV::HWResourceType* ResourceView<TextureUAV>::GetData(u32 mip, u32 slice) const
    {
        TextureUAV::ResourceType* resource = dynamic_cast<TextureUAV::ResourceType*>(m_ResourceScheduler.GetResource(m_ResourceID));
        ATOM_ENGINE_ASSERT(resource, "Resource does not exist or is not the correct type for the view!");
        ATOM_ENGINE_ASSERT(resource->IsAllocated(), "Resource not allocated!");
        auto* view = resource->GetView(mip, slice);
        ATOM_ENGINE_ASSERT(view);
        ATOM_ENGINE_ASSERT(IsSet(view->GetFlags(), TextureUAV::RequiredFlags));
        return view;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    template<>
    typename TextureSRV::HWResourceType* ResourceView<TextureSRV>::GetData() const
    {
        return GetData(TextureView::AllMips, TextureView::AllSlices);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    template<>
    typename TextureSRV::HWResourceType* ResourceView<TextureSRV>::GetData(u32 mip, u32 slice) const
    {
        TextureSRV::ResourceType* resource = dynamic_cast<TextureSRV::ResourceType*>(m_ResourceScheduler.GetResource(m_ResourceID));
        ATOM_ENGINE_ASSERT(resource, "Resource does not exist or is not the correct type for the view!");
        ATOM_ENGINE_ASSERT(resource->IsAllocated(), "Resource not allocated!");
        auto* view = resource->GetView(mip, slice);
        ATOM_ENGINE_ASSERT(view);
        ATOM_ENGINE_ASSERT(IsSet(view->GetFlags(), TextureSRV::RequiredFlags));
        return view;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    template<>
    typename SurfaceRTV::HWResourceType* ResourceView<SurfaceRTV>::GetData() const
    {
        return GetData(TextureView::AllMips, TextureView::AllSlices);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    template<>
    typename SurfaceRTV::HWResourceType* ResourceView<SurfaceRTV>::GetData(u32 mip, u32 slice) const
    {
        SurfaceRTV::ResourceType* resource = dynamic_cast<SurfaceRTV::ResourceType*>(m_ResourceScheduler.GetResource(m_ResourceID));
        ATOM_ENGINE_ASSERT(resource, "Resource does not exist or is not the correct type for the view!");
        ATOM_ENGINE_ASSERT(resource->IsAllocated(), "Resource not allocated!");
        auto* view = resource->GetView(mip, slice);
        ATOM_ENGINE_ASSERT(view);
        ATOM_ENGINE_ASSERT(IsSet(view->GetFlags(), SurfaceRTV::RequiredFlags));
        return view;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    template<>
    typename SurfaceDSV_RW::HWResourceType* ResourceView<SurfaceDSV_RW>::GetData() const
    {
        return GetData(TextureView::AllMips, TextureView::AllSlices);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    template<>
    typename SurfaceDSV_RW::HWResourceType* ResourceView<SurfaceDSV_RW>::GetData(u32 mip, u32 slice) const
    {
        SurfaceDSV_RW::ResourceType* resource = dynamic_cast<SurfaceDSV_RW::ResourceType*>(m_ResourceScheduler.GetResource(m_ResourceID));
        ATOM_ENGINE_ASSERT(resource, "Resource does not exist or is not the correct type for the view!");
        ATOM_ENGINE_ASSERT(resource->IsAllocated(), "Resource not allocated!");
        auto* view = resource->GetView(mip, slice);
        ATOM_ENGINE_ASSERT(view);
        ATOM_ENGINE_ASSERT(IsSet(view->GetFlags(), SurfaceDSV_RW::RequiredFlags));
        return view;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    template<>
    typename SurfaceDSV_RO::HWResourceType* ResourceView<SurfaceDSV_RO>::GetData() const
    {
        return GetData(TextureView::AllMips, TextureView::AllSlices);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    template<>
    typename SurfaceDSV_RO::HWResourceType* ResourceView<SurfaceDSV_RO>::GetData(u32 mip, u32 slice) const
    {
        SurfaceDSV_RO::ResourceType* resource = dynamic_cast<SurfaceDSV_RO::ResourceType*>(m_ResourceScheduler.GetResource(m_ResourceID));
        ATOM_ENGINE_ASSERT(resource, "Resource does not exist or is not the correct type for the view!");
        ATOM_ENGINE_ASSERT(resource->IsAllocated(), "Resource not allocated!");
        auto* view = resource->GetView(mip, slice);
        ATOM_ENGINE_ASSERT(view);
        ATOM_ENGINE_ASSERT(IsSet(view->GetFlags(), SurfaceDSV_RO::RequiredFlags));
        return view;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    template<>
    typename SurfaceSRV::HWResourceType* ResourceView<SurfaceSRV>::GetData() const
    {
        return GetData(TextureView::AllMips, TextureView::AllSlices);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    template<>
    typename SurfaceSRV::HWResourceType* ResourceView<SurfaceSRV>::GetData(u32 mip, u32 slice) const
    {
        SurfaceSRV::ResourceType* resource = dynamic_cast<SurfaceSRV::ResourceType*>(m_ResourceScheduler.GetResource(m_ResourceID));
        ATOM_ENGINE_ASSERT(resource, "Resource does not exist or is not the correct type for the view!");
        ATOM_ENGINE_ASSERT(resource->IsAllocated(), "Resource not allocated!");
        auto* view = resource->GetView(mip, slice);
        ATOM_ENGINE_ASSERT(view);
        ATOM_ENGINE_ASSERT(IsSet(view->GetFlags(), SurfaceSRV::RequiredFlags));
        return view->GetTexture().get();
    }
}