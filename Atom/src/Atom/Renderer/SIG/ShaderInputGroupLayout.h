#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Core/DirectX12/DirectX12.h"
#include "Atom/Renderer/TextureSampler.h"

namespace Atom
{
    class ShaderInputGroupBindPoint
    {
    public:
        inline u32 GetBindingIndex() const { return m_Description.BindIndex; }
        inline u32 GetConstantBufferSize() const { return m_Description.ConstantBufferSize; }
        inline u32 GetNumSRVs() const { return m_Description.NumSRVs; }
        inline u32 GetNumUAVs() const { return m_Description.NumUAVs; }
        inline u32 GetNumSamplers() const { return m_Description.NumSamplers; }
        inline u32 GetConstantBufferRootIdx() const { return m_Description.ConstantBufferRootIndex; }
        inline u32 GetResourceTableRootIdx() const { return m_Description.ResourceTableRootIndex; }
        inline u32 GetSamplerTableRootIdx() const { return m_Description.SamplerTableRootIndex; }

    protected:
        struct Description
        {
            u32 BindIndex;
            u32 ConstantBufferSize;
            u32 NumSRVs;
            u32 NumUAVs;
            u32 NumSamplers;
            u32 ConstantBufferRootIndex;
            u32 ResourceTableRootIndex;
            u32 SamplerTableRootIndex;

            template<typename SIGBindPointType>
            static Description Create()
            {
                Description desc;
                desc.BindIndex = SIGBindPointType::ShaderSpace;
                desc.ConstantBufferSize = SIGBindPointType::ConstantBufferSize;
                desc.NumSRVs = SIGBindPointType::NumSRVs;
                desc.NumUAVs = SIGBindPointType::NumUAVs;
                desc.NumSamplers = SIGBindPointType::NumSamplers;
                desc.ConstantBufferRootIndex = SIGBindPointType::ConstantBufferRootIndex;
                desc.ResourceTableRootIndex = SIGBindPointType::ResourceTableRootIndex;
                desc.SamplerTableRootIndex = SIGBindPointType::SamplerTableRootIndex;
                return desc;
            }
        };
    protected:
        ShaderInputGroupBindPoint(const Description& description)
            : m_Description(description) {}
    protected:
        Description m_Description;
    };

    class ShaderInputGroupStaticSampler
    {
    public:
        inline u32 GetShaderSpace() const { return m_ShaderSpace; }
        inline TextureFilter GetFilter() const { return m_Filter; }
        inline TextureWrap GetWrap() const { return m_Wrap; }
    protected:
        ShaderInputGroupStaticSampler(u32 shaderSpace, TextureFilter filter, TextureWrap wrap)
            : m_ShaderSpace(shaderSpace), m_Filter(filter), m_Wrap(wrap) {}
    protected:
        u32 m_ShaderSpace;
        TextureFilter m_Filter;
        TextureWrap m_Wrap;
    };

    class ShaderInputGroupLayout
    {
    public:
        inline u32 GetNumBindPoints() const { return m_BindPoints.size(); }
        inline const Vector<const ShaderInputGroupBindPoint*>& GetBindPoints() const { return m_BindPoints; }
        inline const Vector<const ShaderInputGroupStaticSampler*>& GetStaticSamplers() const { return m_StaticSamplers; }
        inline ComPtr<ID3D12RootSignature> GetGraphicsRootSignature() const { return m_GraphisRootSignature; }
        inline ComPtr<ID3D12RootSignature> GetComputeRootSignature() const { return m_ComputeRootSignature; }
    protected:
        ShaderInputGroupLayout(const Vector<const ShaderInputGroupBindPoint*>& bindPoints, const Vector<const ShaderInputGroupStaticSampler*>& staticSamplers);
    protected:
        Vector<const ShaderInputGroupBindPoint*>     m_BindPoints;
        Vector<const ShaderInputGroupStaticSampler*> m_StaticSamplers;
        ComPtr<ID3D12RootSignature>                  m_GraphisRootSignature;
        ComPtr<ID3D12RootSignature>                  m_ComputeRootSignature;
    };
}