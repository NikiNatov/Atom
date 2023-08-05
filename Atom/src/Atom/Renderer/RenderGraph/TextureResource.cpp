#include "atompch.h"
#include "TextureResource.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    TextureResource::TextureResource(ResourceID_UA id, const TextureDescription& description)
        : Resource(id, false), m_Description(description), m_ExternalTexture(nullptr)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    TextureResource::TextureResource(ResourceID_UA id, Texture* externalTexture)
        : Resource(id, true), m_ExternalTexture(externalTexture)
    {
        ATOM_ENGINE_ASSERT(m_ExternalTexture);
        m_Description = m_ExternalTexture->GetDescription();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    TextureResource::~TextureResource()
    {
        if (IsAllocated())
            Free();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void TextureResource::Allocate()
    {
        Data* data = new Data();

        data->MainResource = m_ExternalTexture ? m_ExternalTexture : new Texture(m_Description, fmt::format("{}_MainResource", GetName()).c_str());
        
        data->MipViews = new Texture*[m_Description.MipLevels];
        for (u32 mip = 0; mip < m_Description.MipLevels; mip++)
        {
            data->MipViews[mip] = new Texture(*data->MainResource, mip, TextureView::AllSlices);
        }
        
        data->SliceViews = new Texture*[m_Description.ArraySize];
        for (u32 slice = 0; slice < m_Description.ArraySize; slice++)
        {
            data->SliceViews[slice] = new Texture(*data->MainResource, TextureView::AllMips, slice);
        }
        
        data->SubresourceViews = new Texture*[m_Description.MipLevels * m_Description.ArraySize];
        for (u32 slice = 0; slice < m_Description.ArraySize; slice++)
        {
            for (u32 mip = 0; mip < m_Description.MipLevels; mip++)
            {
                u32 subresource = Texture::CalculateSubresource(mip, slice, m_Description.MipLevels, m_Description.ArraySize);
                data->SubresourceViews[subresource] = new Texture(*data->MainResource, mip, slice);
            }
        }

        m_Data = data;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void TextureResource::Free()
    {
        Data* data = (Data*)m_Data;

        if (!m_ExternalTexture)
            delete data->MainResource;

        for (u32 mip = 0; mip < m_Description.MipLevels; mip++)
            delete data->MipViews[mip];

        delete data->MipViews;

        for (u32 slice = 0; slice < m_Description.ArraySize; slice++)
            delete data->SliceViews[slice];

        delete data->SliceViews;

        for (u32 subresource = 0; subresource < m_Description.ArraySize * m_Description.MipLevels; subresource++)
            delete data->SubresourceViews[subresource];

        delete data->SubresourceViews;

        delete m_Data;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Texture* TextureResource::GetView(const Data* data, u32 mip, u32 slice)
    {
        if (mip == TextureView::AllMips && slice == TextureView::AllSlices)
        {
            return data->MainResource;
        }
        else if (mip != TextureView::AllMips && slice == TextureView::AllSlices)
        {
            return data->MipViews[mip];
        }
        else if (mip == TextureView::AllMips && slice != TextureView::AllSlices)
        {
            return data->SliceViews[mip];
        }
        else if (mip != TextureView::AllMips && slice != TextureView::AllSlices)
        {
            return data->SubresourceViews[mip];
        }

        return nullptr;
    }
}
