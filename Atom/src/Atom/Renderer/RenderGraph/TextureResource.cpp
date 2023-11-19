#include "atompch.h"
#include "TextureResource.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    TextureResource::TextureResource(const ResourceID_UA& id, const ResourceDescType& description)
        : Resource(id, false), m_Description(description), m_ExternalResource(nullptr), m_ViewData(nullptr)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    TextureResource::TextureResource(const ResourceID_UA& id, HWResourceType* externalResource)
        : Resource(id, true), m_Description(externalResource->GetDescription()), m_ExternalResource(externalResource), m_ViewData(nullptr)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    TextureResource::~TextureResource()
    {
        Free();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void TextureResource::Allocate()
    {
        if (IsAllocated())
            return;

        m_ViewData = new ViewData();

        m_ViewData->MainResource = m_ExternalResource ? m_ExternalResource : new HWResourceType(m_Description, fmt::format("{}_MainResource", GetName()).c_str());
        
        m_ViewData->MipViews = new HWResourceType*[m_Description.MipLevels];
        for (u32 mip = 0; mip < m_Description.MipLevels; mip++)
        {
            m_ViewData->MipViews[mip] = new HWResourceType(*m_ViewData->MainResource, mip, TextureView::AllSlices);
        }
        
        m_ViewData->SliceViews = new HWResourceType*[m_Description.ArraySize];
        for (u32 slice = 0; slice < m_Description.ArraySize; slice++)
        {
            m_ViewData->SliceViews[slice] = new HWResourceType(*m_ViewData->MainResource, TextureView::AllMips, slice);
        }
        
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
    void TextureResource::Free()
    {
        if (!IsAllocated())
            return;

        if (!m_ExternalResource)
            delete m_ViewData->MainResource;

        for (u32 mip = 0; mip < m_Description.MipLevels; mip++)
            delete m_ViewData->MipViews[mip];

        delete m_ViewData->MipViews;

        for (u32 slice = 0; slice < m_Description.ArraySize; slice++)
            delete m_ViewData->SliceViews[slice];

        delete m_ViewData->SliceViews;

        for (u32 subresource = 0; subresource < m_Description.ArraySize * m_Description.MipLevels; subresource++)
            delete m_ViewData->SubresourceViews[subresource];

        delete m_ViewData->SubresourceViews;

        delete m_ViewData;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool TextureResource::IsAllocated() const
    {
        return m_ViewData != nullptr;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool TextureResource::CanDecayToCommonStateFromState(ResourceState state) const
    {
        return IsSet(state, ResourceState::NonPixelShaderRead) ||
            IsSet(state, ResourceState::PixelShaderRead) ||
            IsSet(state, ResourceState::CopySource);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool TextureResource::CanPromoteFromCommonStateToState(ResourceState state) const
    {
        return IsSet(state, ResourceState::NonPixelShaderRead) || 
            IsSet(state, ResourceState::PixelShaderRead) || 
            IsSet(state, ResourceState::CopySource) || 
            IsSet(state, ResourceState::CopyDestination);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    HWResource* TextureResource::GetHWResource() const
    {
        return IsAllocated() ? m_ViewData->MainResource : nullptr;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Texture* TextureResource::GetView(u32 mip, u32 slice) const
    {
        if (mip == TextureView::AllMips && slice == TextureView::AllSlices)
        {
            return m_ViewData->MainResource;
        }
        else if (mip != TextureView::AllMips && slice == TextureView::AllSlices)
        {
            return m_ViewData->MipViews[mip];
        }
        else if (mip == TextureView::AllMips && slice != TextureView::AllSlices)
        {
            return m_ViewData->SliceViews[mip];
        }
        else if (mip != TextureView::AllMips && slice != TextureView::AllSlices)
        {
            return m_ViewData->SubresourceViews[mip];
        }

        return nullptr;
    }
}
