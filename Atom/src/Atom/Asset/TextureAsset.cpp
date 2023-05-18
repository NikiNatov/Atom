#include "atompch.h"
#include "TextureAsset.h"

#include "Atom/Renderer/Renderer.h"

namespace Atom
{
    // ------------------------------------------------ TextureAsset -----------------------------------------------------------
    // -----------------------------------------------------------------------------------------------------------------------------
    TextureAsset::TextureAsset(AssetType type, const TextureDescription& description, bool cpuReadable)
        : Asset(type), m_CpuReadable(cpuReadable)
    {
        m_TextureResource = CreateRef<Texture>(description, fmt::format("TextureAsset_{:#x}", m_MetaData.UUID).c_str());

        u32 subresourceCount = description.Type == TextureType::Texture3D ? description.MipLevels : description.MipLevels * description.ArraySize;
        m_PixelData.resize(subresourceCount);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    TextureAsset::TextureAsset(AssetType type, const TextureDescription& description, bool cpuReadable, const Vector<Vector<byte>>& pixelData)
        : Asset(type), m_CpuReadable(cpuReadable)
    {
        // Create resource
        m_TextureResource = CreateRef<Texture>(description, fmt::format("TextureAsset_{:#x}", m_MetaData.UUID).c_str());

        // Copy the data if any is provided
        u32 subresourceCount = description.Type == TextureType::Texture3D ? description.MipLevels : description.MipLevels * description.ArraySize;
        ATOM_ENGINE_ASSERT(pixelData.size() == subresourceCount);

        m_PixelData.resize(subresourceCount);

        if (description.Type == TextureType::Texture3D)
        {
            for (u32 mip = 0; mip < description.MipLevels; mip++)
            {
                u32 subresource = D3D12CalcSubresource(mip, 0, 0, description.MipLevels, 1);
                Renderer::UploadTextureData(pixelData[subresource].data(), m_TextureResource, mip);

                if (m_CpuReadable)
                    m_PixelData[subresource] = pixelData[subresource];
            }
        }
        else
        {
            for (u32 slice = 0; slice < description.ArraySize; slice++)
            {
                for (u32 mip = 0; mip < description.MipLevels; mip++)
                {
                    u32 subresource = D3D12CalcSubresource(mip, slice, 0, description.MipLevels, description.ArraySize);
                    Renderer::UploadTextureData(pixelData[subresource].data(), m_TextureResource, mip, slice);

                    if (m_CpuReadable)
                        m_PixelData[subresource] = pixelData[subresource];
                }
            }
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    TextureAsset::TextureAsset(AssetType type, const Ref<Texture>& textureResource, bool cpuReadable)
        : Asset(type), m_TextureResource(textureResource), m_CpuReadable(cpuReadable)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void TextureAsset::UpdateGPUData(bool makeNonReadable)
    {
        if (m_CpuReadable)
        {
            m_CpuReadable = !makeNonReadable;

            if (m_PixelData.empty())
                return;

            if (m_TextureResource->GetType() == TextureType::Texture3D)
            {
                for (u32 mip = 0; mip < m_TextureResource->GetMipLevels(); mip++)
                {
                    u32 subresource = Texture::CalculateSubresource(mip, 0, m_TextureResource->GetMipLevels(), 1);
                    Renderer::UploadTextureData(m_PixelData[subresource].data(), m_TextureResource, mip);

                    if (makeNonReadable)
                        m_PixelData[subresource].clear();
                }
            }
            else
            {
                for (u32 slice = 0; slice < m_TextureResource->GetArraySize(); slice++)
                {
                    for (u32 mip = 0; mip < m_TextureResource->GetMipLevels(); mip++)
                    {
                        u32 subresource = Texture::CalculateSubresource(mip, slice, m_TextureResource->GetMipLevels(), m_TextureResource->GetArraySize());
                        Renderer::UploadTextureData(m_PixelData[subresource].data(), m_TextureResource, mip, slice);

                        if (makeNonReadable)
                            m_PixelData[subresource].clear();
                    }
                }
            }
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool TextureAsset::IsCpuReadable() const
    {
        return m_CpuReadable;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool TextureAsset::IsGpuWritable() const
    {
        return IsSet(m_TextureResource->GetFlags() & TextureFlags::UnorderedAccess);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void TextureAsset::SetFilter(TextureFilter filter)
    {
        m_TextureResource->SetFilter(filter);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void TextureAsset::SetWrap(TextureWrap wrap)
    {
        m_TextureResource->SetWrap(wrap);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    TextureFormat TextureAsset::GetFormat() const
    {
        return m_TextureResource->GetFormat();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 TextureAsset::GetWidth() const
    {
        return m_TextureResource->GetWidth();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 TextureAsset::GetHeight() const
    {
        return m_TextureResource->GetHeight();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 TextureAsset::GetDepth() const
    {
        return m_TextureResource->GetDepth();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 TextureAsset::GetArraySize() const
    {
        return m_TextureResource->GetArraySize();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 TextureAsset::GetMipLevels() const
    {
        return m_TextureResource->GetMipLevels();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    TextureFilter TextureAsset::GetFilter() const
    {
        return m_TextureResource->GetFilter();;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    TextureWrap TextureAsset::GetWrap() const
    {
        return m_TextureResource->GetWrap();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<Texture> TextureAsset::GetResource() const
    {
        return m_TextureResource;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void TextureAsset::SetSubresourcePixels(const Vector<byte>& pixels, u32 subresource)
    {
        if (m_CpuReadable)
            m_PixelData[subresource] = pixels;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const Vector<byte>& TextureAsset::GetSubresourcePixels(u32 subresource) const
    {
        if (m_CpuReadable)
            return m_PixelData[subresource];

        return {};
    }

    // --------------------------------------------------- Texture2D ---------------------------------------------------------------
    // -----------------------------------------------------------------------------------------------------------------------------
    Texture2D::Texture2D(u32 width, u32 height, TextureFormat format, u32 mipCount, bool cpuReadable, bool gpuWritable)
        : TextureAsset(AssetType::Texture2D, {
            TextureType::Texture2D,
            format,
            width,
            height,
            1,
            1,
            mipCount == 0 ? Texture::CalculateMaxMipCount(width, height) : mipCount,
            gpuWritable ? TextureFlags::UnorderedAccess : TextureFlags::DefaultFlags
        }, cpuReadable)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Texture2D::Texture2D(u32 width, u32 height, TextureFormat format, u32 mipCount, bool cpuReadable, bool gpuWritable, const Vector<Vector<byte>>& pixelData)
        : TextureAsset(AssetType::Texture2D, {
            TextureType::Texture2D,
            format,
            width,
            height,
            1,
            1,
            mipCount == 0 ? Texture::CalculateMaxMipCount(width, height) : mipCount,
            gpuWritable ? TextureFlags::UnorderedAccess | TextureFlags::ShaderResource : TextureFlags::ShaderResource
        }, cpuReadable, pixelData)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Texture2D::Texture2D(const Ref<Texture>& textureResource, bool cpuReadable)
        : TextureAsset(AssetType::Texture2D, textureResource, cpuReadable)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Texture2D::SetPixels(const Vector<byte>& pixels, u32 mip)
    {
        u32 subresource = Texture::CalculateSubresource(mip, 0, m_TextureResource->GetMipLevels(), 1);
        SetSubresourcePixels(pixels, subresource);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const Vector<byte>& Texture2D::GetPixels(u32 mip) const
    {
        u32 subresource = Texture::CalculateSubresource(mip, 0, m_TextureResource->GetMipLevels(), 1);
        return GetSubresourcePixels(subresource);
    }

    // --------------------------------------------------- TextureCube -------------------------------------------------------------
    // -----------------------------------------------------------------------------------------------------------------------------
    TextureCube::TextureCube(u32 size, TextureFormat format, u32 mipCount, bool cpuReadable, bool gpuWritable)
        : TextureAsset(AssetType::TextureCube, {
            TextureType::Texture2D,
            format,
            size,
            size,
            1,
            6,
            mipCount == 0 ? Texture::CalculateMaxMipCount(size, size) : mipCount,
            gpuWritable ? TextureFlags::UnorderedAccess | TextureFlags::ShaderResource | TextureFlags::CubeMap : TextureFlags::ShaderResource | TextureFlags::CubeMap
        }, cpuReadable)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    TextureCube::TextureCube(u32 size, TextureFormat format, u32 mipCount, bool cpuReadable, bool gpuWritable, const Vector<Vector<byte>>& pixelData)
        : TextureAsset(AssetType::TextureCube, {
            TextureType::Texture2D,
            format,
            size,
            size,
            1,
            6,
            mipCount == 0 ? Texture::CalculateMaxMipCount(size, size) : mipCount,
            gpuWritable ? TextureFlags::UnorderedAccess | TextureFlags::ShaderResource | TextureFlags::CubeMap : TextureFlags::ShaderResource | TextureFlags::CubeMap
        }, cpuReadable, pixelData)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    TextureCube::TextureCube(const Ref<Texture>& textureResource, bool cpuReadable)
        : TextureAsset(AssetType::TextureCube, textureResource, cpuReadable)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void TextureCube::SetPixels(const Vector<byte>& pixels, u32 mip, u32 face)
    {
        u32 subresource = Texture::CalculateSubresource(mip, face, m_TextureResource->GetMipLevels(), 6);
        SetSubresourcePixels(pixels, subresource);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const Vector<byte>& TextureCube::GetPixels(u32 mip, u32 face) const
    {
        u32 subresource = Texture::CalculateSubresource(mip, face, m_TextureResource->GetMipLevels(), 6);
        return GetSubresourcePixels(subresource);
    }
}
