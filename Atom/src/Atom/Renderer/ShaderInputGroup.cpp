#include "atompch.h"
#include "ShaderInputGroup.h"

#include "Atom/Renderer/Device.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    CustomShaderInputGroup::CustomShaderInputGroup(ShaderBindPoint bindPoint, u32 constantBufferSize, u32 numResources, u32 numSamplers)
        : m_BindPoint(bindPoint)
    {
        m_ConstantsData.resize(constantBufferSize);
        m_ResourceDescriptors.resize(numResources);
        m_SamplerDescriptors.resize(numSamplers);

        if (m_BindPoint == ShaderBindPoint::Frame || m_BindPoint == ShaderBindPoint::Pass)
        {
            BufferDescription desc;
            desc.ElementCount = 1;
            desc.ElementSize = 256 * ((constantBufferSize + 255) / 256);
            desc.IsDynamic = true;

            m_ConstantBuffer = CreateRef<ConstantBuffer>(desc, "CustomSIG_ConstantBuffer");
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    CustomShaderInputGroup::~CustomShaderInputGroup()
    {
        if (m_BindPoint == ShaderBindPoint::Material)
        {
            if (m_ResourceTable.IsValid())
                Device::Get().GetGPUDescriptorHeap(DescriptorHeapType::ShaderResource)->Release(std::move(m_ResourceTable), true);
            if (m_SamplerTable.IsValid())
                Device::Get().GetGPUDescriptorHeap(DescriptorHeapType::Sampler)->Release(std::move(m_SamplerTable), true);
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CustomShaderInputGroup::SetConstantData(u32 offset, const void* data, u32 dataSize)
    {
        ATOM_ENGINE_ASSERT(offset + dataSize <= m_ConstantsData.size());
        memcpy(m_ConstantsData.data() + offset, data, dataSize);
        m_ConstantsDirty = true;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CustomShaderInputGroup::SetROTexture(u32 index, const Ref<Texture>& texture)
    {
        ATOM_ENGINE_ASSERT(index < m_ResourceDescriptors.size());
        m_ResourceDescriptors[index] = texture->GetSRV()->GetDescriptor();
        m_ResourcesDirty = true;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CustomShaderInputGroup::SetRWTexture(u32 index, const Ref<Texture>& texture)
    {
        ATOM_ENGINE_ASSERT(index < m_ResourceDescriptors.size());
        m_ResourceDescriptors[index] = texture->GetUAV()->GetDescriptor();
        m_ResourcesDirty = true;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CustomShaderInputGroup::SetROBuffer(u32 index, const Ref<StructuredBuffer>& buffer)
    {
        ATOM_ENGINE_ASSERT(index < m_ResourceDescriptors.size());
        m_ResourceDescriptors[index] = buffer->GetSRV();
        m_ResourcesDirty = true;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CustomShaderInputGroup::SetRWBuffer(u32 index, const Ref<StructuredBuffer>& buffer)
    {
        ATOM_ENGINE_ASSERT(false, "Not implemented");
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CustomShaderInputGroup::SetSampler(u32 index, const Ref<TextureSampler>& sampler)
    {
        ATOM_ENGINE_ASSERT(index < m_SamplerDescriptors.size());
        m_SamplerDescriptors[index] = sampler->GetDescriptor();
        m_SamplersDirty = true;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CustomShaderInputGroup::Compile()
    {
        if (m_ConstantBuffer && m_ConstantsDirty)
        {
            void* data = m_ConstantBuffer->Map(0, 0);
            memcpy(data, m_ConstantsData.data(), m_ConstantsData.size());
            m_ConstantBuffer->Unmap();
            
            m_ConstantsDirty = false;
        }

        if (m_ResourcesDirty)
        {
            if (!m_ResourceTable.IsValid())
            {
                if(m_BindPoint == ShaderBindPoint::Material)
                    m_ResourceTable = Device::Get().GetGPUDescriptorHeap(DescriptorHeapType::ShaderResource)->AllocatePersistent(m_ResourceDescriptors.size());
                else
                    m_ResourceTable = Device::Get().GetGPUDescriptorHeap(DescriptorHeapType::ShaderResource)->AllocateTransient(m_ResourceDescriptors.size());
            }

            Device::Get().CopyDescriptors(m_ResourceTable, m_ResourceDescriptors.size(), m_ResourceDescriptors.data(), DescriptorHeapType::ShaderResource);
            m_ResourcesDirty = false;
        }

        if (m_SamplersDirty)
        {
            if (!m_SamplerTable.IsValid())
            {
                if (m_BindPoint == ShaderBindPoint::Material)
                    m_SamplerTable = Device::Get().GetGPUDescriptorHeap(DescriptorHeapType::Sampler)->AllocatePersistent(m_SamplerDescriptors.size());
                else
                    m_SamplerTable = Device::Get().GetGPUDescriptorHeap(DescriptorHeapType::Sampler)->AllocateTransient(m_SamplerDescriptors.size());
            }

            Device::Get().CopyDescriptors(m_SamplerTable, m_SamplerDescriptors.size(), m_SamplerDescriptors.data(), DescriptorHeapType::Sampler);
            m_SamplersDirty = false;
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    ShaderBindPoint CustomShaderInputGroup::GetBindPoint() const
    {
        return m_BindPoint;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const byte* CustomShaderInputGroup::GetRootConstantsData() const
    {
        return m_BindPoint == ShaderBindPoint::Material || m_BindPoint == ShaderBindPoint::Instance ? m_ConstantsData.data() : nullptr;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const Ref<ConstantBuffer>& CustomShaderInputGroup::GetConstantBuffer() const
    {
        return m_BindPoint == ShaderBindPoint::Material || m_BindPoint == ShaderBindPoint::Instance ? nullptr : m_ConstantBuffer;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const DescriptorAllocation& CustomShaderInputGroup::GetResourceTable() const
    {
        return m_ResourceTable;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const DescriptorAllocation& CustomShaderInputGroup::GetSamplerTable() const
    {
        return m_SamplerTable;
    }
}
