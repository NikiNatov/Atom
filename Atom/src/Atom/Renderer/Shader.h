#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Core/DirectX12/DirectX12.h"
#include "Atom/Renderer/DescriptorHeap.h"

#include <d3dcompiler.h>

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

    enum class ShaderInlineDescriptorType
    {
        None = 0,
        CBV, SRV, UAV
    };

    class ShaderResourceLayout
    {
    public:
        struct ShaderUniform
        {
            String Name;
            ShaderDataType Type;
            u32 Offset;
            u32 Size;
            u32 BufferRegister;
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
            ShaderResourceType Type;
            u32 Register;
            u32 ShaderSpace;

            ShaderResource(const String& name, ShaderResourceType type, u32 shaderRegister, u32 shaderSpace)
                : Name(name), Type(type), Register(shaderRegister), ShaderSpace(shaderSpace) {}

            inline bool IsReadOnly() const { return !(Type == ShaderResourceType::RWTexture2D || Type == ShaderResourceType::RWTexture2DArray || Type == ShaderResourceType::RWStructuredBuffer); }
        };

        struct ShaderInlineDescriptor
        {
            u32 RootParameterIndex;
            ShaderInlineDescriptorType Type;

            ShaderInlineDescriptor(u32 rootParamIdx, ShaderInlineDescriptorType type)
                : RootParameterIndex(rootParamIdx), Type(type) {}
        };

        struct ShaderDescriptorTable
        {
            u32 RootParameterIndex;
            DescriptorHeapType Type;
            u32 DescriptorCount;

            ShaderDescriptorTable(u32 rootParamIdx, DescriptorHeapType type, u32 descriptorCount)
                : RootParameterIndex(rootParamIdx), Type(type), DescriptorCount(descriptorCount) {}
        };
    public:
        ShaderResourceLayout() = default;
        ShaderResourceLayout(const ComPtr<ID3DBlob>& vsDataBlob, const ComPtr<ID3DBlob>& psDataBlob);
        ShaderResourceLayout(const ComPtr<ID3DBlob>& csDataBlob);
        ~ShaderResourceLayout();

        inline const Vector<ConstantBuffer>& GetRootConstants() const { return m_RootConstants; }
        inline const Vector<ShaderResource>& GetResources() const { return m_Resources; }
        inline const Vector<ShaderInlineDescriptor>& GetInlineDescriptors() const { return m_InlineDescriptors; }
        inline const Vector<ShaderDescriptorTable>& GetDescriptorTables() const { return m_DescriptorTables; }
        inline ComPtr<ID3D12RootSignature> GetRootSignature() const { return m_RootSignature; }
    private:
        void Reflect(const ComPtr<ID3DBlob>& shaderDataBlob);
        void CreateRootSignature();
    private:
        Vector<ConstantBuffer>         m_RootConstants;
        Vector<ShaderResource>         m_Resources;
        Vector<ShaderInlineDescriptor> m_InlineDescriptors;
        Vector<ShaderDescriptorTable>  m_DescriptorTables;
        ComPtr<ID3D12RootSignature>    m_RootSignature;
    };

    class Shader
    {
    public:
        virtual ~Shader() = default;

        inline const String& GetName() const { return m_Name; }
        inline const std::filesystem::path& GetFilepath() const { return m_Filepath; }
        inline u64 GetHash() const { return std::hash<String>{}(m_Filepath.string()); }
        inline const ShaderResourceLayout& GetResourceLayout() const { return m_ResourceLayout; }
    protected:
        Shader(const std::filesystem::path& filepath);
    protected:
        String                m_Name;
        std::filesystem::path m_Filepath;
        ShaderResourceLayout  m_ResourceLayout;
    };

    class GraphicsShader : public Shader
    {
    public:
        GraphicsShader(const std::filesystem::path& filepath);
        ~GraphicsShader();

        inline ComPtr<ID3DBlob> GetVSData() const { return m_VSData; }
        inline ComPtr<ID3DBlob> GetPSData() const { return m_PSData; }
    private:
        ComPtr<ID3DBlob> m_VSData;
        ComPtr<ID3DBlob> m_PSData;
    };

    class ComputeShader : public Shader
    {
    public:
        ComputeShader(const std::filesystem::path& filepath);
        ~ComputeShader();

        inline ComPtr<ID3DBlob> GetCSData() const { return m_CSData; }
    private:
        ComPtr<ID3DBlob> m_CSData;
    };
}