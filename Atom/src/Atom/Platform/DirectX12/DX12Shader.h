#pragma once

#include "Atom/Renderer/API/Shader.h"

#if defined(ATOM_PLATFORM_WINDOWS)

#include "DirectX12.h"
#include <d3dcompiler.h>

namespace Atom
{
    class DX12Shader : public Shader
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
        DX12Shader(const String& filepath);
        ~DX12Shader();

        virtual const String& GetName() const override;
        virtual const String& GetFilepath() const override;
        virtual u64 GetHash() const override;

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

#endif // ATOM_PLATFORM_WINDOWS