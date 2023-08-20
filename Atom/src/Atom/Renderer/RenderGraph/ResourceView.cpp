#include "atompch.h"
#include "ResourceView.h"

#include "Atom/Renderer/RenderGraph/RenderGraph.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    template<>
    typename const TextureUAV::ReturnType* ResourceView<TextureResource, TextureUAV>::GetData(u32 mip, u32 slice) const
    {
        auto* data = (TextureResource::Data*)m_Resource->GetData();
        ATOM_ENGINE_ASSERT(data, "Resource not allocated!");
        auto* view = TextureResource::GetView(data, mip, slice);
        ATOM_ENGINE_ASSERT(view);
        ATOM_ENGINE_ASSERT(IsSet(view->GetFlags() & TextureFlags::UnorderedAccess));
        return view;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    template<>
    typename const TextureSRV::ReturnType* ResourceView<TextureResource, TextureSRV>::GetData(u32 mip, u32 slice) const
    {
        auto* data = (TextureResource::Data*)m_Resource->GetData();
        ATOM_ENGINE_ASSERT(data, "Resource not allocated!");
        auto* view = TextureResource::GetView(data, mip, slice);
        ATOM_ENGINE_ASSERT(view);
        ATOM_ENGINE_ASSERT(IsSet(view->GetFlags() & TextureFlags::ShaderResource));
        return view;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    template<>
    typename const SurfaceRTV::ReturnType* ResourceView<RenderSurfaceResource, SurfaceRTV>::GetData(u32 mip, u32 slice) const
    {
        auto* data = (RenderSurfaceResource::Data*)m_Resource->GetData();
        ATOM_ENGINE_ASSERT(data, "Resource not allocated!");
        auto* view = RenderSurfaceResource::GetView(data, mip, slice);
        ATOM_ENGINE_ASSERT(view);
        ATOM_ENGINE_ASSERT(IsSet(view->GetFlags() & TextureFlags::RenderTarget));
        return view;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    template<>
    typename const SurfaceDSV::ReturnType* ResourceView<RenderSurfaceResource, SurfaceDSV>::GetData(u32 mip, u32 slice) const
    {
        auto* data = (RenderSurfaceResource::Data*)m_Resource->GetData();
        ATOM_ENGINE_ASSERT(data, "Resource not allocated!");
        auto* view = RenderSurfaceResource::GetView(data, mip, slice);
        ATOM_ENGINE_ASSERT(view);
        ATOM_ENGINE_ASSERT(IsSet(view->GetFlags() & TextureFlags::DepthStencil));
        return view;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    template<>
    typename const TextureSRV::ReturnType* ResourceView<RenderSurfaceResource, TextureSRV>::GetData(u32 mip, u32 slice) const
    {
        auto data = (RenderSurfaceResource::Data*)m_Resource->GetData();
        ATOM_ENGINE_ASSERT(data, "Resource not allocated!");
        auto* view = RenderSurfaceResource::GetView(data, mip, slice);
        ATOM_ENGINE_ASSERT(view);
        ATOM_ENGINE_ASSERT(IsSet(view->GetFlags() & TextureFlags::ShaderResource));
        return view->GetTexture().get();
    }
}