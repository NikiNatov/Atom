#include "atompch.h"
#include "ResourceView.h"

#include "Atom/Renderer/RenderGraph/RenderGraph.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    template<>
    template<>
    typename const TextureSRV::ReturnType* ResourceView<TextureSRV>::GetData<TextureSRV>(u32 mip, u32 slice) const
    {
        auto data = (TextureSRV::ViewDataType*)m_Graph->GetResource(m_ResourceID)->GetData();
        ATOM_ENGINE_ASSERT(data, "Resource not allocated!");
        auto* view = TextureSRV::ResourceType::GetView(data, mip, slice);
        ATOM_ENGINE_ASSERT(view);
        ATOM_ENGINE_ASSERT(IsSet(view->GetFlags() & TextureFlags::ShaderResource));
        return view;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    template<>
    template<>
    typename const TextureUAV::ReturnType* ResourceView<TextureUAV>::GetData<TextureUAV>(u32 mip, u32 slice) const
    {
        auto data = (TextureUAV::ViewDataType*)m_Graph->GetResource(m_ResourceID)->GetData();
        ATOM_ENGINE_ASSERT(data, "Resource not allocated!");
        auto* view = TextureUAV::ResourceType::GetView(data, mip, slice);
        ATOM_ENGINE_ASSERT(view);
        ATOM_ENGINE_ASSERT(IsSet(view->GetFlags() & TextureFlags::UnorderedAccess));
        return view;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    template<>
    template<>
    typename const TextureSRV::ReturnType* ResourceView<TextureUAV>::GetData<TextureSRV>(u32 mip, u32 slice) const
    {
        auto data = (TextureUAV::ViewDataType*)m_Graph->GetResource(m_ResourceID)->GetData();
        ATOM_ENGINE_ASSERT(data, "Resource not allocated!");
        auto* view = TextureUAV::ResourceType::GetView(data, mip, slice);
        ATOM_ENGINE_ASSERT(view);
        ATOM_ENGINE_ASSERT(IsSet(view->GetFlags() & TextureFlags::ShaderResource));
        return view;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    template<>
    template<>
    typename const SurfaceRTV::ReturnType* ResourceView<SurfaceRTV>::GetData<SurfaceRTV>(u32 mip, u32 slice) const
    {
        auto data = (SurfaceRTV::ViewDataType*)m_Graph->GetResource(m_ResourceID)->GetData();
        ATOM_ENGINE_ASSERT(data, "Resource not allocated!");
        auto* view = SurfaceRTV::ResourceType::GetView(data, mip, slice);
        ATOM_ENGINE_ASSERT(view);
        ATOM_ENGINE_ASSERT(IsSet(view->GetFlags() & TextureFlags::RenderTarget));
        return view;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    template<>
    template<>
    typename const TextureSRV::ReturnType* ResourceView<SurfaceRTV>::GetData<TextureSRV>(u32 mip, u32 slice) const
    {
        auto data = (SurfaceRTV::ViewDataType*)m_Graph->GetResource(m_ResourceID)->GetData();
        ATOM_ENGINE_ASSERT(data, "Resource not allocated!");
        auto* view = SurfaceRTV::ResourceType::GetView(data, mip, slice);
        ATOM_ENGINE_ASSERT(view);
        ATOM_ENGINE_ASSERT(IsSet(view->GetFlags() & TextureFlags::ShaderResource));
        return view->GetTexture().get();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    template<>
    template<>
    typename const SurfaceDSV::ReturnType* ResourceView<SurfaceDSV>::GetData<SurfaceDSV>(u32 mip, u32 slice) const
    {
        auto data = (SurfaceDSV::ViewDataType*)m_Graph->GetResource(m_ResourceID)->GetData();
        ATOM_ENGINE_ASSERT(data, "Resource not allocated!");
        auto* view = SurfaceDSV::ResourceType::GetView(data, mip, slice);
        ATOM_ENGINE_ASSERT(view);
        ATOM_ENGINE_ASSERT(IsSet(view->GetFlags() & TextureFlags::DepthStencil));
        return view;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    template<>
    template<>
    typename const TextureSRV::ReturnType* ResourceView<SurfaceDSV>::GetData<TextureSRV>(u32 mip, u32 slice) const
    {
        auto data = (SurfaceDSV::ViewDataType*)m_Graph->GetResource(m_ResourceID)->GetData();
        ATOM_ENGINE_ASSERT(data, "Resource not allocated!");
        auto* view = SurfaceDSV::ResourceType::GetView(data, mip, slice);
        ATOM_ENGINE_ASSERT(view);
        ATOM_ENGINE_ASSERT(IsSet(view->GetFlags() & TextureFlags::ShaderResource));
        return view->GetTexture().get();
    }
}