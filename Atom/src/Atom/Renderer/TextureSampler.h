#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Core/DirectX12/DirectX12.h"

namespace Atom
{
    enum class TextureFilter
    {
        Linear,
        Nearest,
        Anisotropic,
        NumFilters
    };

    enum class TextureWrap
    {
        Clamp,
        Repeat,
        NumWraps
    };

    class TextureSampler
    {
    public:
        TextureSampler(TextureFilter filter, TextureWrap wrap);
        ~TextureSampler();

        TextureSampler(const TextureSampler& rhs) = delete;
        TextureSampler& operator=(const TextureSampler& rhs) = delete;

        TextureSampler(TextureSampler&& rhs) noexcept;
        TextureSampler& operator=(TextureSampler&& rhs) noexcept;

        TextureFilter GetFilter() const { return m_Filter; }
        TextureWrap GetWrap() const { return m_Wrap; }
        D3D12_CPU_DESCRIPTOR_HANDLE GetDescriptor() const { return m_Descriptor; }
    private:
        TextureFilter               m_Filter;
        TextureWrap                 m_Wrap;
        D3D12_CPU_DESCRIPTOR_HANDLE m_Descriptor{ 0 };
    };
}