#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Core/DirectX12/DirectX12.h"

#include <glm/glm.hpp>

namespace Atom
{
    enum class TextureType
    {
        None = 0,
        Texture2D,
        TextureCube,
        SwapChainBuffer
    };

    enum class TextureFormat
    {
        None = 0,
        RGBA8,
        RG16F, RGBA16F,
        RG32F, RGBA32F,
        Depth24Stencil8,
        Depth32
    };

    enum class TextureFilter
    {
        None = 0,
        Linear,
        Nearest,
        Anisotropic
    };

    enum class TextureWrap
    {
        None = 0,
        Clamp, Repeat
    };

    enum class TextureBindFlags : u8
    {
        None = 0,
        UnorderedAccess = BIT(1),
        RenderTarget = BIT(2),
        DepthStencil = BIT(3)
    };

    IMPL_ENUM_OPERATORS(TextureBindFlags);

    struct DepthStencilValue
    {
        f32 DepthValue;
        u8 StencilValue;

        DepthStencilValue(f32 depth = 1.0f, u8 stencil = 0)
            : DepthValue(depth), StencilValue(stencil) {}
    };

    struct ClearValue
    {
        union
        {
            glm::vec4 Color;
            DepthStencilValue DepthStencil = {};
        };

        ClearValue()
            : Color(0.0f, 0.0f, 0.0f, 1.0f), DepthStencil() {}

        ClearValue(f32 r, f32 g, f32 b, f32 a)
            : Color(r, g, b, a) {}

        ClearValue(f32 depthValue, u8 stencilValue)
            : DepthStencil(depthValue, stencilValue) {}
    };

    class Image2D
    {
    public:
        Image2D(u32 width, u32 height, u32 bytesPerPixel, u32 maxMipCount, bool isHDR, const byte* pixelData);
        Image2D(const std::filesystem::path& filepath, u32 desiredChannels = 4);
        Image2D(const byte* compressedData, u32 dataSize, u32 desiredChannels = 4);

        inline u32 GetWidth() const { return m_Width; }
        inline u32 GetHeight() const { return m_Height; }
        inline u32 GetBytesPerPixel() const { return m_BytesPerPixel; }
        inline u32 GetMaxMipCount() const { return m_MaxMipCount; }
        inline bool IsHDR() const { return m_IsHDR; }
        inline const Vector<byte>& GetPixelData() const { return m_PixelData; }
    private:
        u32          m_Width;
        u32          m_Height;
        u32          m_BytesPerPixel;
        u32          m_MaxMipCount;
        bool         m_IsHDR;
        Vector<byte> m_PixelData;
    };

    struct TextureDescription
    {
        TextureFormat Format = TextureFormat::RGBA8;
        u32 Width = 0;
        u32 Height = 0;
        u32 MipLevels = 1;
        TextureBindFlags UsageFlags = TextureBindFlags::None;
        TextureFilter Filter = TextureFilter::Linear;
        TextureWrap Wrap = TextureWrap::Repeat;
        ClearValue ClearValue = {};
    };

    class Texture
    {
    public:
        Texture(TextureType type, const TextureDescription& description, const char* debugName);
        Texture(TextureType type, ID3D12Resource* textureHandle, const char* debugName);
        virtual ~Texture();

        TextureType GetType() const;
        TextureFormat GetFormat() const;
        u32 GetWidth() const;
        u32 GetHeight() const;
        u32 GetMipLevels() const;
        TextureBindFlags GetBindFlags() const;
        TextureFilter GetFilter() const;
        TextureWrap GetWrap() const;
        const ClearValue& GetClearValue() const;
        inline ComPtr<ID3D12Resource> GetD3DResource() const { return m_D3DResource; }
        inline D3D12_CPU_DESCRIPTOR_HANDLE GetSRV() const { return m_SRVDescriptor; }
        inline D3D12_CPU_DESCRIPTOR_HANDLE GetSampler() const { return m_SamplerDescriptor; }

    protected:
        virtual void CreateViews();
    protected:
        ComPtr<ID3D12Resource>      m_D3DResource;
        D3D12_CPU_DESCRIPTOR_HANDLE m_SRVDescriptor;
        D3D12_CPU_DESCRIPTOR_HANDLE m_SamplerDescriptor;
        TextureType                 m_Type;
        TextureDescription          m_Description;
    };

    class Texture2D : public Texture
    {
    public:
        Texture2D(const TextureDescription& description, const char* debugName = "Unnamed Texture2D");
        Texture2D(ID3D12Resource* textureHandle, const char* debugName = "Unnamed Texture2D");
        ~Texture2D();

        D3D12_CPU_DESCRIPTOR_HANDLE GetUAV(u32 mip = 0) const;
    private:
        virtual void CreateViews() override;
    private:
        Vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_UAVDescriptors;
    };

    class RenderTexture2D : public Texture
    {
    public:
        RenderTexture2D(const TextureDescription& description, bool swapChainBuffer, const char* debugName = "Unnamed Render Texture");
        RenderTexture2D(ID3D12Resource* textureHandle, bool swapChainBuffer, const char* debugName = "Unnamed Render Texture");
        ~RenderTexture2D();

        inline bool IsSwapChainBuffer() const { return m_Type == TextureType::SwapChainBuffer; }
        D3D12_CPU_DESCRIPTOR_HANDLE GetRTV(u32 mip = 0) const;
    private:
        virtual void CreateViews() override;
    private:
        Vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_RTVDescriptors;
    };

    class DepthBuffer : public Texture
    {
    public:
        DepthBuffer(const TextureDescription& description, const char* debugName = "Unnamed Depth Buffer");
        DepthBuffer(ID3D12Resource* textureHandle, const char* debugName = "Unnamed Depth Buffer");
        ~DepthBuffer();

        D3D12_CPU_DESCRIPTOR_HANDLE GetDSV(u32 mip = 0) const;
    private:
        virtual void CreateViews() override;
    private:
        Vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_DSVDescriptors;
    };

    class TextureCube : public Texture
    {
    public:
        TextureCube(const TextureDescription& description, const char* debugName = "Unnamed TextureCube");
        TextureCube(ID3D12Resource* textureHandle, const char* debugName = "Unnamed TextureCube");
        ~TextureCube();

        D3D12_CPU_DESCRIPTOR_HANDLE GetUAV(u32 slice = 0, u32 mip = 0) const;
        D3D12_CPU_DESCRIPTOR_HANDLE GetArrayUAV(u32 mip = 0) const;
    private:
        virtual void CreateViews() override;
    private:
        Vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_UAVDescriptors;
        Vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_ArrayUAVDescriptors;
    };
}