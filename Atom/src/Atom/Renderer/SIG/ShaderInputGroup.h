#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Renderer/Texture.h"
#include "Atom/Renderer/Buffer.h"
#include "Atom/Renderer/TextureSampler.h"
#include "Atom/Renderer/SIG/ShaderInputGroupStorage.h"

namespace Atom
{
    class ShaderInputGroup
    {
    public:
        virtual ~ShaderInputGroup() = default;

        void Compile();

        inline const ShaderInputGroupStorage& GetStorage() const { return m_SIGStorage; }
    protected:
        ShaderInputGroup(const ShaderInputGroupStorage& sigStorage);
    protected:
        ShaderInputGroupStorage m_SIGStorage;
    };

    template<typename SIGType>
    class CustomShaderInputGroup : public ShaderInputGroup
    {
    public:
        typedef typename SIGType::BindPointType BindPointType;
    public:
        CustomShaderInputGroup(u32 constantBufferSize, u32 numSRV, u32 numUAV, u32 numSamplers, bool persistentStorage)
            : ShaderInputGroup(ShaderInputGroupStorage(constantBufferSize, numSRV, numUAV, numSamplers, persistentStorage))
        {
        }

        void SetConstant(u32 offset, const void* data, u32 size)
        {
            byte* constants = m_SIGStorage.GetConstants<byte>();
            memcpy(constants + offset, data, size);
            m_SIGStorage.SetConstantsDirtyFlag(true);
        }

        void SetROTexture(u32 slot, const Texture* texture)
        {
            D3D12_CPU_DESCRIPTOR_HANDLE* srvDescriptors = m_SIGStorage.GetSRVDescriptors<D3D12_CPU_DESCRIPTOR_HANDLE>();
            srvDescriptors[slot] = texture->GetSRV()->GetDescriptor();
            m_SIGStorage.SetResourcesDirtyFlag(true);
        }

        void SetRWTexture(u32 slot, const Texture* texture)
        {
            D3D12_CPU_DESCRIPTOR_HANDLE* uavDescriptors = m_SIGStorage.GetUAVDescriptors<D3D12_CPU_DESCRIPTOR_HANDLE>();
            uavDescriptors[slot] = texture->GetUAV()->GetDescriptor();
            m_SIGStorage.SetResourcesDirtyFlag(true);
        }

        void SetROBuffer(u32 slot, const StructuredBuffer* buffer)
        {
            D3D12_CPU_DESCRIPTOR_HANDLE* srvDescriptors = m_SIGStorage.GetSRVDescriptors<D3D12_CPU_DESCRIPTOR_HANDLE>();
            srvDescriptors[slot] = buffer->GetSRV();
            m_SIGStorage.SetResourcesDirtyFlag(true);
        }

        void SetSampler(u32 slot, const TextureSampler* sampler)
        {
            D3D12_CPU_DESCRIPTOR_HANDLE* samplerDescriptors = m_SIGStorage.GetSamplerDescriptors<D3D12_CPU_DESCRIPTOR_HANDLE>();
            samplerDescriptors[slot] = sampler->GetDescriptor();
            m_SIGStorage.SetSamplersDirtyFlag(true);
        }
    };
}