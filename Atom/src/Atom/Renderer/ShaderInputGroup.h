#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Renderer/DescriptorHeap.h"
#include "Atom/Renderer/ShaderLayout.h"
#include "Atom/Renderer/Buffer.h"
#include "Atom/Renderer/Texture.h"
#include "Atom/Renderer/TextureSampler.h"

namespace Atom
{
    class ShaderInputGroup
    {
    public:
        virtual ~ShaderInputGroup() = default;

        virtual void Compile() = 0;

        virtual ShaderBindPoint GetBindPoint() const = 0;
        virtual const byte* GetRootConstantsData() const = 0;
        virtual const Ref<ConstantBuffer>& GetConstantBuffer() const = 0;
        virtual const DescriptorAllocation& GetResourceTable() const = 0;
        virtual const DescriptorAllocation& GetSamplerTable() const = 0;
    };

    class CustomShaderInputGroup : public ShaderInputGroup
    {
    public:
        CustomShaderInputGroup(ShaderBindPoint bindPoint, u32 constantBufferSize, u32 numResources, u32 numSamplers);
        ~CustomShaderInputGroup();

        void SetConstantData(u32 offset, const void* data, u32 dataSize);
        void SetROTexture(u32 index, const Ref<Texture>& texture);
        void SetRWTexture(u32 index, const Ref<Texture>& texture);
        void SetROBuffer(u32 index, const Ref<StructuredBuffer>& buffer);
        void SetRWBuffer(u32 index, const Ref<StructuredBuffer>& buffer);
        void SetSampler(u32 index, const Ref<TextureSampler>& sampler);

        virtual void Compile() override;

        virtual ShaderBindPoint GetBindPoint() const override;
        virtual const byte* GetRootConstantsData() const override;
        virtual const Ref<ConstantBuffer>& GetConstantBuffer() const override;
        virtual const DescriptorAllocation& GetResourceTable() const override;
        virtual const DescriptorAllocation& GetSamplerTable() const override;

        inline u32 GetRootConstantsCount() const { return m_BindPoint == ShaderBindPoint::Material || m_BindPoint == ShaderBindPoint::Instance ? m_ConstantsData.size() / 4 : 0; }
    private:
        ShaderBindPoint                     m_BindPoint;

        Vector<byte>                        m_ConstantsData;
        Ref<ConstantBuffer>                 m_ConstantBuffer;
        bool                                m_ConstantsDirty = false;

        DescriptorAllocation                m_ResourceTable;
        Vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_ResourceDescriptors;
        bool                                m_ResourcesDirty = false;

        DescriptorAllocation                m_SamplerTable;
        Vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_SamplerDescriptors;
        bool                                m_SamplersDirty = false;
    };
}