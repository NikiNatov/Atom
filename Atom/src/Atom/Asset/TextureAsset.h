#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Asset/Asset.h"
#include "Atom/Renderer/Texture.h"
#include "Atom/Renderer/TextureSampler.h"

namespace Atom
{
    class TextureAsset : public Asset
    {
        friend class AssetSerializer;
    public:
        virtual ~TextureAsset() = default;

        void UpdateGPUData(bool makeNonReadable = false);
        bool IsCpuReadable() const;
        bool IsGpuWritable() const;

        void SetFilter(TextureFilter filter);
        void SetWrap(TextureWrap wrap);

        TextureFormat GetFormat() const;
        u32 GetWidth() const;
        u32 GetHeight() const;
        u32 GetDepth() const;
        u32 GetArraySize() const;
        u32 GetMipLevels() const;
        TextureFilter GetFilter() const;
        TextureWrap GetWrap() const;
        Ref<Texture> GetResource() const;
    protected:
        TextureAsset(AssetType type, const TextureDescription& description, bool cpuReadable);
        TextureAsset(AssetType type, const TextureDescription& description, bool cpuReadable, const Vector<Vector<byte>>& pixelData);
        TextureAsset(AssetType type, const Ref<Texture>& textureResource, bool cpuReadable);

        void SetSubresourcePixels(const Vector<byte>& pixels, u32 subresource);
        const Vector<byte>& GetSubresourcePixels(u32 subresource) const;
    protected:
        Ref<Texture>         m_TextureResource;
        Vector<Vector<byte>> m_PixelData;
        bool                 m_CpuReadable;
        TextureFilter        m_Filter = TextureFilter::Linear;
        TextureWrap          m_Wrap = TextureWrap::Repeat;
    };

    class Texture2D : public TextureAsset
    {
        friend class AssetSerializer;
    public:
        Texture2D(u32 width, u32 height, TextureFormat format, u32 mipCount, bool cpuReadable, bool gpuWritable);
        Texture2D(u32 width, u32 height, TextureFormat format, u32 mipCount, bool cpuReadable, bool gpuWritable, const Vector<Vector<byte>>& pixelData);
        Texture2D(const Ref<Texture>& textureResource, bool cpuReadable);

        void SetPixels(const Vector<byte>& pixels, u32 mip);
        const Vector<byte>& GetPixels(u32 mip) const;
    };

    class TextureCube : public TextureAsset
    {
        friend class AssetSerializer;
    public:
        TextureCube(u32 size, TextureFormat format, u32 mipCount, bool cpuReadable, bool gpuWritable);
        TextureCube(u32 size, TextureFormat format, u32 mipCount, bool cpuReadable, bool gpuWritable, const Vector<Vector<byte>>& pixelData);
        TextureCube(const Ref<Texture>& textureResource, bool cpuReadable);

        void SetPixels(const Vector<byte>& pixels, u32 mip, u32 face);
        const Vector<byte>& GetPixels(u32 mip, u32 face) const;
    };
}