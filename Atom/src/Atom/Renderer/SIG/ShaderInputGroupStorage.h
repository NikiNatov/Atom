#pragma once

#include "Atom/Core/Core.h"

#include "Atom/Renderer/DescriptorHeap.h"
#include "Atom/Renderer/Buffer.h"
#include "Atom/Renderer/Device.h"

#include "Atom/Renderer/SIG/ShaderInputGroupLayout.h"

namespace Atom
{
    class ShaderInputGroupStorage
    {
    public:
        ShaderInputGroupStorage(u32 constantBufferSize, u32 numSRV, u32 numUAV, u32 numSamplers, bool persistentStorage);
        ~ShaderInputGroupStorage();

        void CopyDescriptors();

        template<typename ConstantsStructType>
        typename ConstantsStructType* GetConstants()
        {
            ATOM_ENGINE_ASSERT(m_ConstantBufferData.size() >= sizeof(ConstantsStructType), "Constants struct size is too big");
            return (ConstantsStructType*)m_ConstantBufferData.data();
        }

        template<typename SRVStructType>
        typename SRVStructType* GetSRVDescriptors()
        {
            ATOM_ENGINE_ASSERT(m_NumSRV >= sizeof(SRVStructType) / sizeof(D3D12_CPU_DESCRIPTOR_HANDLE), "SRV struct size is too big");
            return (SRVStructType*)m_CPUDescriptors.data();
        }

        template<typename UAVStructType>
        typename UAVStructType* GetUAVDescriptors()
        {
            ATOM_ENGINE_ASSERT(m_NumUAV >= sizeof(UAVStructType) / sizeof(D3D12_CPU_DESCRIPTOR_HANDLE), "UAV struct size is too big");
            return (UAVStructType*)(m_CPUDescriptors.data() + m_NumSRV);
        }

        template<typename SamplersStructType>
        typename SamplersStructType* GetSamplerDescriptors()
        {
            ATOM_ENGINE_ASSERT(m_NumSamplers >= sizeof(SamplersStructType) / sizeof(D3D12_CPU_DESCRIPTOR_HANDLE), "Sampler struct size is too big");
            return (SamplersStructType*)(m_CPUDescriptors.data() + m_NumSRV + m_NumUAV);
        }

        inline void SetConstantsDirtyFlag(bool value) { m_ConstantsDirty = value; }
        inline void SetResourcesDirtyFlag(bool value) { m_ResourcesDirty = value; }
        inline void SetSamplersDirtyFlag(bool value) { m_SamplersDirty = value; }

        inline const Ref<ConstantBuffer>& GetConstantBuffer() const { return m_ConstantBuffer; }
        inline const DescriptorAllocation& GetResourceTable() const { return m_ResourceTable; }
        inline const DescriptorAllocation& GetSamplerTable() const { return m_SamplerTable; }

        template<typename SIGType>
        static ShaderInputGroupStorage Create(bool persistentStorage)
        {
            u32 constantBufferSize = sizeof(SIGType::Constants);
            u32 numSRVs = sizeof(SIGType::SRVForLayout) / sizeof(D3D12_CPU_DESCRIPTOR_HANDLE);
            u32 numUAVs = sizeof(SIGType::UAVForLayout) / sizeof(D3D12_CPU_DESCRIPTOR_HANDLE);
            u32 numSamplers = sizeof(SIGType::SamplersForLayout) / sizeof(D3D12_CPU_DESCRIPTOR_HANDLE);
            return ShaderInputGroupStorage(constantBufferSize, numSRVs, numUAVs, numSamplers, persistentStorage);
        }
    private:
        u32                                 m_NumSRV;
        u32                                 m_NumUAV;
        u32                                 m_NumSamplers;
        bool                                m_PersistentStorage;

        Vector<byte>                        m_ConstantBufferData;
        Ref<ConstantBuffer>                 m_ConstantBuffer;
        Vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_CPUDescriptors;
        DescriptorAllocation                m_ResourceTable;
        DescriptorAllocation                m_SamplerTable;
        bool                                m_ConstantsDirty = false;
        bool                                m_ResourcesDirty = false;
        bool                                m_SamplersDirty = false;
    };
}