#include "atompch.h"
#include "TextureSampler.h"

#include "Atom/Core/DirectX12/DirectX12Utils.h"
#include "Atom/Renderer/Device.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    TextureSampler::TextureSampler(TextureFilter filter, TextureWrap wrap)
        : m_Filter(filter), m_Wrap(wrap)
    {
        auto& dx12Device = Device::Get();

        D3D12_SAMPLER_DESC samplerDesc = {};
        samplerDesc.AddressU = Utils::AtomTextureWrapToD3D12(wrap);
        samplerDesc.AddressV = Utils::AtomTextureWrapToD3D12(wrap);
        samplerDesc.AddressW = Utils::AtomTextureWrapToD3D12(wrap);
        samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        samplerDesc.Filter = Utils::AtomTextureFilterToD3D12(filter);
        samplerDesc.MinLOD = 0.0f;
        samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
        samplerDesc.MaxAnisotropy = filter == TextureFilter::Anisotropic ? D3D12_REQ_MAXANISOTROPY : 1;

        m_Descriptor = dx12Device.GetCPUDescriptorHeap(DescriptorHeapType::Sampler)->AllocateDescriptor();
        dx12Device.GetD3DDevice()->CreateSampler(&samplerDesc, m_Descriptor);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    TextureSampler::~TextureSampler()
    {
        Device::Get().GetCPUDescriptorHeap(DescriptorHeapType::Sampler)->ReleaseDescriptor(m_Descriptor, true);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    TextureSampler::TextureSampler(TextureSampler&& rhs) noexcept
        : m_Descriptor(std::move(rhs.m_Descriptor)), m_Filter(rhs.m_Filter), m_Wrap(rhs.m_Wrap)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    TextureSampler& TextureSampler::operator=(TextureSampler&& rhs) noexcept
    {
        if (this != &rhs)
        {
            Device::Get().GetCPUDescriptorHeap(DescriptorHeapType::Sampler)->ReleaseDescriptor(m_Descriptor, true);
            m_Descriptor = std::move(rhs.m_Descriptor);
            m_Filter = rhs.m_Filter;
            m_Wrap = rhs.m_Wrap;
        }

        return *this;
    }
}
