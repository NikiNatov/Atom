#include "atompch.h"
#include "RenderSurfaceResource.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    RenderSurfaceResource::RenderSurfaceResource(const ResourceID_RT& id, const ResourceDescType& description)
        : Resource(id, false), m_Description(description), m_ExternalResource(nullptr), m_ViewData(nullptr)
    {
        ATOM_ENGINE_ASSERT(IsSet(m_Description.Flags, TextureFlags::RenderTarget));
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    RenderSurfaceResource::RenderSurfaceResource(const ResourceID_DS& id, const ResourceDescType& description)
        : Resource(id, false), m_Description(description), m_ExternalResource(nullptr), m_ViewData(nullptr)
    {
        ATOM_ENGINE_ASSERT(IsSet(m_Description.Flags, TextureFlags::DepthStencil));
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    RenderSurfaceResource::RenderSurfaceResource(const ResourceID_RT& id, HWResourceType* externalResource)
        : Resource(id, true), m_Description(externalResource->GetTexture()->GetDescription()), m_ExternalResource(externalResource), m_ViewData(nullptr)
    {
        ATOM_ENGINE_ASSERT(IsSet(m_ExternalResource->GetFlags(), TextureFlags::RenderTarget));
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    RenderSurfaceResource::RenderSurfaceResource(const ResourceID_DS& id, HWResourceType* externalResource)
        : Resource(id, true), m_Description(externalResource->GetTexture()->GetDescription()), m_ExternalResource(externalResource), m_ViewData(nullptr)
    {
        ATOM_ENGINE_ASSERT(IsSet(m_ExternalResource->GetFlags(), TextureFlags::DepthStencil));
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    RenderSurfaceResource::~RenderSurfaceResource()
    {
        Free();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderSurfaceResource::Allocate()
    {
        if (IsAllocated())
            return;

        m_ViewData = new ViewData();

        m_ViewData->MainResource = m_ExternalResource ? m_ExternalResource : new HWResourceType(m_Description, fmt::format("{}_MainResource", GetName()).c_str());

        m_ViewData->SubresourceViews = new HWResourceType*[m_Description.MipLevels * m_Description.ArraySize];
        for (u32 slice = 0; slice < m_Description.ArraySize; slice++)
        {
            for (u32 mip = 0; mip < m_Description.MipLevels; mip++)
            {
                u32 subresource = Texture::CalculateSubresource(mip, slice, m_Description.MipLevels, m_Description.ArraySize);
                m_ViewData->SubresourceViews[subresource] = new HWResourceType(*m_ViewData->MainResource, mip, slice);
            }
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void RenderSurfaceResource::Free()
    {
        if (!IsAllocated())
            return;

        if (!m_ExternalResource)
            delete m_ViewData->MainResource;

        for (u32 subresource = 0; subresource < m_Description.ArraySize * m_Description.MipLevels; subresource++)
            delete m_ViewData->SubresourceViews[subresource];

        delete m_ViewData->SubresourceViews;

        delete m_ViewData;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool RenderSurfaceResource::IsAllocated() const
    {
        return m_ViewData != nullptr;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool RenderSurfaceResource::CanDecayToCommonStateFromState(ResourceState state) const
    {
        return IsSet(state, ResourceState::NonPixelShaderRead) ||
            IsSet(state, ResourceState::PixelShaderRead) ||
            IsSet(state, ResourceState::CopySource);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool RenderSurfaceResource::CanPromoteFromCommonStateToState(ResourceState state) const
    {
        return IsSet(state, ResourceState::NonPixelShaderRead) ||
            IsSet(state, ResourceState::PixelShaderRead) ||
            IsSet(state, ResourceState::CopySource) ||
            IsSet(state, ResourceState::CopyDestination);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    HWResource* RenderSurfaceResource::GetHWResource() const
    {
        return IsAllocated() ? m_ViewData->MainResource->GetTexture().get() : nullptr;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    RenderSurface* RenderSurfaceResource::GetView(u32 mip, u32 slice) const
    {
        if (mip == TextureView::AllMips && slice == TextureView::AllSlices)
        {
            return m_ViewData->MainResource;
        }
        else if (mip != TextureView::AllMips && slice != TextureView::AllSlices)
        {
            u32 subresource = Texture::CalculateSubresource(mip, slice, m_Description.MipLevels, m_Description.ArraySize);
            return m_ViewData->SubresourceViews[subresource];
        }

        return nullptr;
    }
}
