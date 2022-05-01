#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Core/DirectX12/DirectX12.h"

#include <d3dcompiler.h>

namespace Atom
{
    enum class ShaderDataType
    {
        Int, Int2, Int3, Int4,
        Uint, Uint2, Uint3, Uint4,
        Float, Float2, Float3, Float4,
        Bool,
        Mat2, Mat3, Mat4
    };

    class Shader
    {
    public:
        struct ShaderUniform
        {
            String Name;
            D3D_SHADER_VARIABLE_TYPE Type;
            u32 Offset;
            u32 Size;
        };

        struct ConstantBuffer
        {
            String Name;
            u32 Register;
            u32 ShaderSpace;
            u32 Size;
            Vector<ShaderUniform> Uniforms;
        };

        struct ShaderResource
        {
            String Name;
            D3D_SHADER_INPUT_TYPE Type;
            u32 Register;
            u32 ShaderSpace;
        };

        struct ShaderDescriptorTable
        {
            HashMap<u32, ShaderResource> ConstantBuffers;
            HashMap<u32, ShaderResource> ResourcesRO;
            HashMap<u32, ShaderResource> ResourcesRW;
            HashMap<u32, ShaderResource> Samplers;
        };

    public:
        Shader(const String& filepath);
        ~Shader();

        const String& GetName() const;
        const String& GetFilepath() const;
        u64 GetHash() const;

        inline const ShaderDescriptorTable& GetDescriptorTable() const { return m_DescriptorTable; }
        inline ComPtr<ID3DBlob> GetVSData() const { return m_VSData; }
        inline ComPtr<ID3DBlob> GetPSData() const { return m_PSData; }
        inline ComPtr<ID3D12RootSignature> GetRootSignature() const { return m_RootSignature; }
    private:
        String ReadFile(const String& filepath);
        void Reflect(const ComPtr<ID3DBlob>& shaderDataBlob);
        void CreateRootSignature();
    private:
        String                          m_Name;
        String                          m_Filepath;
        ComPtr<ID3DBlob>                m_VSData;
        ComPtr<ID3DBlob>                m_PSData;
        ComPtr<ID3D12RootSignature>     m_RootSignature;

        ShaderDescriptorTable           m_DescriptorTable;
        HashMap<String, ConstantBuffer> m_Constants;
        HashMap<String, ShaderResource> m_Resources;
    };
}