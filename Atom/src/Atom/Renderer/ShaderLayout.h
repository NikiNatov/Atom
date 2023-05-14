#pragma once

#include "Atom/Core/Core.h"

namespace Atom
{
    enum class ShaderDataType
    {
        None = 0,
        Unorm4,
        Int, Int2, Int3, Int4,
        Uint, Uint2, Uint3, Uint4,
        Float, Float2, Float3, Float4,
        Bool,
        Mat2, Mat3, Mat4
    };

    enum class ShaderResourceType
    {
        None = 0,
        ConstantBuffer,
        Texture2D, Texture2DArray, TextureCube, RWTexture2D, RWTexture2DArray,
        StructuredBuffer, RWStructuredBuffer,
        Sampler
    };

    enum class ShaderBindPoint
    {
        Instance,
        Material,
        Pass,
        Frame,
        NumBindPoints
    };

    class ShaderLayout
    {
    public:
        struct Uniform
        {
            String Name;
            ShaderDataType Type;
            u32 Register;
            u32 Offset;
            u32 Size;

            Uniform(const String& name, ShaderDataType type, u32 bufferRegister, u32 offset, u32 size)
                : Name(name), Type(type), Register(bufferRegister), Offset(offset), Size(size) {}
        };

        struct ShaderResource
        {
            String Name;
            ShaderResourceType Type;
            u32 Register;

            ShaderResource(const String& name, ShaderResourceType type, u32 shaderRegister)
                : Name(name), Type(type), Register(shaderRegister) {}
        };

        struct ShaderConstants
        {
            ShaderBindPoint BindPoint;
            u32 RootParameterIndex = UINT32_MAX;
            u32 Register = UINT32_MAX;
            u32 TotalSize = 0;
            Vector<Uniform> Uniforms;
        };

        struct ShaderDescriptorTable
        {
            ShaderBindPoint BindPoint;
            u32 RootParameterIndex = UINT32_MAX;
            Vector<ShaderResource> Resources;
        };

    public:
        ShaderLayout();
        ShaderLayout(const ComPtr<ID3DBlob>& vsDataBlob, const ComPtr<ID3DBlob>& psDataBlob);
        ShaderLayout(const ComPtr<ID3DBlob>& csDataBlob);
        ~ShaderLayout();

        inline const ShaderConstants& GetConstants(ShaderBindPoint bindPoint) const { return m_Constants[(u32)bindPoint]; }
        inline const ShaderDescriptorTable& GetResourceDescriptorTable(ShaderBindPoint bindPoint) const { return m_ResourceDescriptorTables[(u32)bindPoint]; }
        inline const ShaderDescriptorTable& GetSamplerDescriptorTable(ShaderBindPoint bindPoint) const { return m_SamplerDescriptorTables[(u32)bindPoint]; }
        inline ComPtr<ID3D12RootSignature> GetRootSignature() const { return m_RootSignature; }
    private:
        void Reflect(const ComPtr<ID3DBlob>& shaderDataBlob);
        void CreateRootSignature();
        void LogReflectionInfo();
    private:
        ShaderConstants             m_Constants[u32(ShaderBindPoint::NumBindPoints)];
        ShaderDescriptorTable       m_ResourceDescriptorTables[u32(ShaderBindPoint::NumBindPoints)];
        ShaderDescriptorTable       m_SamplerDescriptorTables[u32(ShaderBindPoint::NumBindPoints)];
        ComPtr<ID3D12RootSignature> m_RootSignature;
    };
}