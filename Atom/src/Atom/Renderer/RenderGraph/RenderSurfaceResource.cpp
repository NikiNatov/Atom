#include "atompch.h"
#include "RenderSurfaceResource.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    RenderSurfaceResource::RenderSurfaceResource(ResourceID_RT id, const TextureDescription& description)
        : Resource(id, false), m_Description(description), m_ExternalSurface(nullptr)
    {
        ATOM_ENGINE_ASSERT(IsSet(m_Description.Flags & TextureFlags::RenderTarget));
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    RenderSurfaceResource::RenderSurfaceResource(ResourceID_DS id, const TextureDescription& description)
        : Resource(id, false), m_Description(description), m_ExternalSurface(nullptr)
    {
        ATOM_ENGINE_ASSERT(IsSet(m_Description.Flags & TextureFlags::DepthStencil));
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    RenderSurfaceResource::RenderSurfaceResource(ResourceID_RT id, RenderSurface* externalSurface)
        : Resource(id, true), m_ExternalSurface(externalSurface)
    {
        ATOM_ENGINE_ASSERT(m_ExternalSurface && IsSet(m_ExternalSurface->GetFlags() & TextureFlags::RenderTarget));
        m_Description = m_ExternalSurface->GetTexture()->GetDescription();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    RenderSurfaceResource::RenderSurfaceResource(ResourceID_DS id, RenderSurface* externalSurface)
        : Resource(id, true), m_ExternalSurface(externalSurface)
    {
        ATOM_ENGINE_ASSERT(m_ExternalSurface && IsSet(m_ExternalSurface->GetFlags() & TextureFlags::DepthStencil));
        m_Description = m_ExternalSurface->GetTexture()->GetDescription();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    RenderSurfaceResource::~RenderSurfaceResource()
    {
        if (IsAllocated())
            Free();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderSurfaceResource::Allocate()
    {
        Data* data = new Data();

        data->MainResource = m_ExternalSurface ? m_ExternalSurface : new RenderSurface(m_Description, fmt::format("{}_MainResource", GetName()).c_str());

        data->SubresourceViews = new RenderSurface*[m_Description.MipLevels * m_Description.ArraySize];
        for (u32 slice = 0; slice < m_Description.ArraySize; slice++)
        {
            for (u32 mip = 0; mip < m_Description.MipLevels; mip++)
            {
                u32 subresource = Texture::CalculateSubresource(mip, slice, m_Description.MipLevels, m_Description.ArraySize);
                data->SubresourceViews[subresource] = new RenderSurface(*data->MainResource, mip, slice);
            }
        }

        m_Data = data;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderSurfaceResource::Free()
    {
        Data* data = (Data*)m_Data;

        if (!m_ExternalSurface)
            delete data->MainResource;

        for (u32 subresource = 0; subresource < m_Description.ArraySize * m_Description.MipLevels; subresource++)
            delete data->SubresourceViews[subresource];

        delete data->SubresourceViews;

        delete m_Data;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool RenderSurfaceResource::CanDecayToCommonStateFromState(ResourceState state) const
    {
        return IsSet(state & ResourceState::NonPixelShaderRead) ||
            IsSet(state & ResourceState::PixelShaderRead) ||
            IsSet(state & ResourceState::CopySource);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool RenderSurfaceResource::CanPromoteFromCommonStateToState(ResourceState state) const
    {
        return IsSet(state & ResourceState::NonPixelShaderRead) ||
            IsSet(state & ResourceState::PixelShaderRead) ||
            IsSet(state & ResourceState::CopySource) ||
            IsSet(state & ResourceState::CopyDestination);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    RenderSurface* RenderSurfaceResource::GetView(const Data* data, u32 mip, u32 slice)
    {
        if (mip == TextureView::AllMips && slice == TextureView::AllSlices)
        {
            return data->MainResource;
        }
        else if (mip != TextureView::AllMips && slice != TextureView::AllSlices)
        {
            return data->SubresourceViews[mip];
        }

        return nullptr;
    }
}
