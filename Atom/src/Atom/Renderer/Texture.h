#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Core/DirectX12/DirectX12.h"
#include "Atom/Asset/Asset.h"

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
        R8,
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

        void SetFilter(TextureFilter filter);
        void SetWrap(TextureWrap wrap);

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
        void CreateSRV();
        void CreateSampler();
    protected:
        ComPtr<ID3D12Resource>      m_D3DResource;
        D3D12_CPU_DESCRIPTOR_HANDLE m_SRVDescriptor{ 0 };
        D3D12_CPU_DESCRIPTOR_HANDLE m_SamplerDescriptor{ 0 };
        TextureType                 m_Type;
        TextureDescription          m_Description;
    };

    class Texture2D : public Texture, public Asset
    {
        friend class AssetSerializer;
        friend class ContentTools;
    public:
        Texture2D(const TextureDescription& description, const Vector<Vector<byte>>& pixelData, bool isReadable, const char* debugName = "Unnamed Texture2D");
        Texture2D(ID3D12Resource* textureHandle, const char* debugName = "Unnamed Texture2D");
        ~Texture2D();

        void UpdateGPUData(bool makeNonReadable = false);
        void SetPixels(const Vector<byte>& pixels, u32 mipLevel = 0);
        Vector<byte>* GetPixels(u32 mipLevel = 0);
        bool IsReadable() const;

        D3D12_CPU_DESCRIPTOR_HANDLE GetUAV(u32 mip = 0) const;
    private:
        void CreateUAV();
    private:
        Vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_UAVDescriptors;
        Vector<Vector<byte>>                m_PixelData;
        bool                                m_IsReadable;
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
        void CreateRTV();
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
        void CreateDSV();
    private:
        Vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_DSVDescriptors;
    };

    class TextureCube : public Texture, public Asset
    {
        friend class AssetSerializer;
        friend class ContentTools;
    public:
        TextureCube(const TextureDescription& description, Vector<Vector<byte>> pixelData[6], bool isReadable, const char* debugName = "Unnamed TextureCube");
        TextureCube(ID3D12Resource* textureHandle, const char* debugName = "Unnamed TextureCube");
        ~TextureCube();

        void UpdateGPUData(bool makeNonReadable = false);
        void SetPixels(const Vector<byte>& pixels, u32 cubeFace, u32 mipLevel = 0);
        Vector<byte>* GetPixels(u32 cubeFace, u32 mipLevel = 0);
        bool IsReadable() const;

        D3D12_CPU_DESCRIPTOR_HANDLE GetUAV(u32 slice = 0, u32 mip = 0) const;
        D3D12_CPU_DESCRIPTOR_HANDLE GetArrayUAV(u32 mip = 0) const;
    private:
        void CreateUAV();
    private:
        Vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_UAVDescriptors;
        Vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_ArrayUAVDescriptors;
        Vector<Vector<byte>>                m_PixelData[6];
        bool                                m_IsReadable;
    };
}