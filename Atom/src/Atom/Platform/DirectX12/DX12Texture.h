#pragma once

#include "Atom/Renderer/API/Texture.h"

#if defined(ATOM_PLATFORM_WINDOWS)

#include "DirectX12.h"
#include "DX12DescriptorAllocator.h"

namespace Atom
{
    class DX12Texture : public Texture
    {
    public:
        DX12Texture(TextureType type, const TextureDescription& description, const char* debugName);
        DX12Texture(TextureType type, u64 textureHandle, const char* debugName);
        ~DX12Texture();

        virtual void Release() override;
        virtual void DeferredRelease() override;
        virtual TextureType GetType() const override;
        virtual TextureFormat GetFormat() const override;
        virtual u32 GetWidth() const override;
        virtual u32 GetHeight() const override;
        virtual u32 GetMipLevels() const override;
        virtual u8 GetUsageFlags() const override;
        virtual TextureFilter GetFilter() const override;
        virtual TextureWrap GetWrap() const override;
        
        inline ComPtr<ID3D12Resource> GetD3DResource() const { return m_D3DResource; }
    private:
        ComPtr<ID3D12Resource> m_D3DResource;
        TextureType            m_Type;
        TextureDescription     m_Description;
    };
}

#endif // ATOM_PLATFORM_WINDOWS