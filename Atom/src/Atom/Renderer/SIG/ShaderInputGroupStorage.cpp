#include "atompch.h"
#include "ShaderInputGroupStorage.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    ShaderInputGroupStorage::ShaderInputGroupStorage(u32 constantBufferSize, u32 numSRV, u32 numUAV, u32 numSamplers, bool persistentStorage)
        : m_NumSRV(numSRV), m_NumUAV(numUAV), m_NumSamplers(numSamplers), m_PersistentStorage(persistentStorage)
    {
        GPUDescriptorHeap* resourceHeap = Device::Get().GetGPUDescriptorHeap(DescriptorHeapType::ShaderResource);
        GPUDescriptorHeap* samplerHeap = Device::Get().GetGPUDescriptorHeap(DescriptorHeapType::Sampler);

        m_ConstantBufferData.resize(constantBufferSize, 0);

        BufferDescription cbDesc;
        cbDesc.ElementSize = ((constantBufferSize + 255) / 256) * 256;
        cbDesc.ElementCount = 1;
        cbDesc.IsDynamic = true;

        m_ConstantBuffer = CreateRef<ConstantBuffer>(cbDesc);

        m_CPUDescriptors.resize(m_NumSRV + m_NumUAV + m_NumSamplers, D3D12_CPU_DESCRIPTOR_HANDLE{ 0 });
        m_ResourceTable = m_PersistentStorage ? resourceHeap->AllocatePersistent(m_NumSRV + m_NumUAV) : resourceHeap->AllocateTransient(m_NumSRV + m_NumUAV);
        m_SamplerTable = m_PersistentStorage ? samplerHeap->AllocatePersistent(m_NumSamplers) : samplerHeap->AllocateTransient(m_NumSamplers);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    ShaderInputGroupStorage::~ShaderInputGroupStorage()
    {
        if (m_PersistentStorage)
        {
            Device::Get().GetGPUDescriptorHeap(DescriptorHeapType::ShaderResource)->Release(std::move(m_ResourceTable), true);
            Device::Get().GetGPUDescriptorHeap(DescriptorHeapType::Sampler)->Release(std::move(m_SamplerTable), true);
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ShaderInputGroupStorage::CopyDescriptors()
    {
        if (m_ConstantsDirty)
        {
            void* data = m_ConstantBuffer->Map(0, 0);
            memcpy(data, m_ConstantBufferData.data(), m_ConstantBufferData.size());
            m_ConstantBuffer->Unmap();

            m_ConstantsDirty = false;
        }

        auto CopyDescriptors = [](const DescriptorAllocation& gpuTable, DescriptorHeapType heapType, D3D12_CPU_DESCRIPTOR_HANDLE* cpuDescriptors, u32 count)
        {
            u32 startOffset = UINT32_MAX;
            u32 numDescriptorsToCopy = 0;
            for (u32 i = 0; i < count; i++)
            {
                if (cpuDescriptors[i].ptr != 0)
                {
                    if (startOffset == UINT32_MAX)
                        startOffset = i;

                    numDescriptorsToCopy++;

                    if (i == count - 1)
                        Device::Get().CopyDescriptors(gpuTable, startOffset, numDescriptorsToCopy, &cpuDescriptors[startOffset], heapType);
                }
                else
                {
                    if (numDescriptorsToCopy > 0)
                    {
                        Device::Get().CopyDescriptors(gpuTable, startOffset, numDescriptorsToCopy, &cpuDescriptors[startOffset], heapType);
                        startOffset = UINT32_MAX;
                        numDescriptorsToCopy = 0;
                    }
                }
            }
        };

        if (m_ResourcesDirty)
        {
            D3D12_CPU_DESCRIPTOR_HANDLE* resourceCPUDescriptors = m_CPUDescriptors.data();
            CopyDescriptors(m_ResourceTable, DescriptorHeapType::ShaderResource, resourceCPUDescriptors, m_NumSRV + m_NumUAV);
            m_ResourcesDirty = false;
        }

        if (m_SamplersDirty)
        {
            D3D12_CPU_DESCRIPTOR_HANDLE* samplerCPUDescriptors = m_CPUDescriptors.data() + m_NumSRV + m_NumUAV;
            CopyDescriptors(m_SamplerTable, DescriptorHeapType::Sampler, samplerCPUDescriptors, m_NumSamplers);
            m_SamplersDirty = false;
        }
    }
}
