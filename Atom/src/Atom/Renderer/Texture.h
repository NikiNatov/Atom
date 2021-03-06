#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Core/DirectX12/DirectX12.h"

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

    struct TextureDescription
    {
        TextureFormat Format = TextureFormat::RGBA8;
        u32 Width = 0;
        u32 Height = 0;
        u32 MipLevels = 1;
        TextureBindFlags UsageFlags = TextureBindFlags::None;
        TextureFilter Filter = TextureFilter::Linear;
        TextureWrap Wrap = TextureWrap::Repeat;
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
        inline ComPtr<ID3D12Resource> GetD3DResource() const { return m_D3DResource; }
    protected:
        virtual void CreateViews() = 0;
    protected:
        ComPtr<ID3D12Resource> m_D3DResource;
        TextureType            m_Type;
        TextureDescription     m_Description;
    };

    class Texture2D : public Texture
    {
    public:
        Texture2D(const TextureDescription& description, const char* debugName = "Unnamed Texture2D");
        Texture2D(ID3D12Resource* textureHandle, const char* debugName = "Unnamed Texture2D");
        ~Texture2D();

        D3D12_CPU_DESCRIPTOR_HANDLE GetSRV() const;
        D3D12_CPU_DESCRIPTOR_HANDLE GetUAV(u32 mip = 0) const;
    protected:
        virtual void CreateViews() override;
    protected:
        D3D12_CPU_DESCRIPTOR_HANDLE         m_SRVDescriptor;
        Vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_UAVDescriptors;
    };

    class RenderTexture2D : public Texture
    {
    public:
        RenderTexture2D(const TextureDescription& description, bool swapChainBuffer, const char* debugName = "Unnamed Render Texture");
        RenderTexture2D(ID3D12Resource* textureHandle, bool swapChainBuffer, const char* debugName = "Unnamed Render Texture");
        ~RenderTexture2D();

        inline bool IsSwapChainBuffer() const { return m_Type == TextureType::SwapChainBuffer; }
        D3D12_CPU_DESCRIPTOR_HANDLE GetSRV() const;
        D3D12_CPU_DESCRIPTOR_HANDLE GetRTV(u32 mip = 0) const;
    private:
        virtual void CreateViews() override;
    private:
        D3D12_CPU_DESCRIPTOR_HANDLE         m_SRVDescriptor;
        Vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_RTVDescriptors;
    };

    class DepthBuffer : public Texture
    {
    public:
        DepthBuffer(const TextureDescription& description, const char* debugName = "Unnamed Depth Buffer");
        DepthBuffer(ID3D12Resource* textureHandle, const char* debugName = "Unnamed Depth Buffer");
        ~DepthBuffer();

        D3D12_CPU_DESCRIPTOR_HANDLE GetSRV() const;
        D3D12_CPU_DESCRIPTOR_HANDLE GetDSV(u32 mip = 0) const;
    private:
        virtual void CreateViews() override;
    private:
        D3D12_CPU_DESCRIPTOR_HANDLE         m_SRVDescriptor;
        Vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_DSVDescriptors;
    };
}