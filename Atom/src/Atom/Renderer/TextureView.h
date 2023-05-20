#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Core/DirectX12/DirectX12.h"
#include "Atom/Renderer/DescriptorHeap.h"

namespace Atom
{
    namespace TextureView
    {
        static constexpr u32 AllMips = 0xFFFFFFFF;
        static constexpr u32 AllSlices = 0xFFFFFFFF;
    }

    struct TextureViewDescription
    {
        u32 FirstMip;
        u32 MipLevels;
        u32 FirstSlice;
        u32 ArraySize;
    };

    class Texture;
    class RenderSurface;

    class TextureViewRO
    {
    public:
        using ResourceType = Texture;
    public:
        TextureViewRO(const Texture* resource, const TextureViewDescription& description, bool deferredRelease = true);
        ~TextureViewRO();

        TextureViewRO(const TextureViewRO& rhs) = delete;
        TextureViewRO& operator=(const TextureViewRO& rhs) = delete;

        TextureViewRO(TextureViewRO&& rhs) noexcept;
        TextureViewRO& operator=(TextureViewRO&& rhs) noexcept;

        D3D12_CPU_DESCRIPTOR_HANDLE GetDescriptor() const { return m_Descriptor; }
    private:
        bool                        m_DeferredRelease = true;
        D3D12_CPU_DESCRIPTOR_HANDLE m_Descriptor{ 0 };
    };

    class TextureViewRW
    {
    public:
        using ResourceType = Texture;
    public:
        TextureViewRW(const Texture* resource, const TextureViewDescription& description, bool deferredRelease = true);
        ~TextureViewRW();

        TextureViewRW(const TextureViewRW& rhs) = delete;
        TextureViewRW& operator=(const TextureViewRW& rhs) = delete;

        TextureViewRW(TextureViewRW&& rhs) noexcept;
        TextureViewRW& operator=(TextureViewRW&& rhs) noexcept;

        D3D12_CPU_DESCRIPTOR_HANDLE GetDescriptor() const { return m_Descriptor; }
    private:
        bool                        m_DeferredRelease = true;
        D3D12_CPU_DESCRIPTOR_HANDLE m_Descriptor{ 0 };
    };

    class TextureViewRT
    {
    public:
        using ResourceType = RenderSurface;
    public:
        TextureViewRT(const RenderSurface* resource, const TextureViewDescription& description, bool deferredRelease = true);
        ~TextureViewRT();

        TextureViewRT(const TextureViewRT& rhs) = delete;
        TextureViewRT& operator=(const TextureViewRT& rhs) = delete;

        TextureViewRT(TextureViewRT&& rhs) noexcept;
        TextureViewRT& operator=(TextureViewRT&& rhs) noexcept;

        D3D12_CPU_DESCRIPTOR_HANDLE GetDescriptor() const { return m_Descriptor; }
    private:
        bool                        m_DeferredRelease = true;
        D3D12_CPU_DESCRIPTOR_HANDLE m_Descriptor{ 0 };
    };

    class TextureViewDS
    {
    public:
        using ResourceType = RenderSurface;
    public:
        TextureViewDS(const RenderSurface* resource, const TextureViewDescription& description, bool deferredRelease = true);
        ~TextureViewDS();

        TextureViewDS(const TextureViewDS& rhs) = delete;
        TextureViewDS& operator=(const TextureViewDS& rhs) = delete;

        TextureViewDS(TextureViewDS&& rhs) noexcept;
        TextureViewDS& operator=(TextureViewDS&& rhs) noexcept;

        D3D12_CPU_DESCRIPTOR_HANDLE GetDescriptor() const { return m_Descriptor; }
    private:
        bool                        m_DeferredRelease = true;
        D3D12_CPU_DESCRIPTOR_HANDLE m_Descriptor{ 0 };
    };
}