#include "atompch.h"

#if defined(ATOM_PLATFORM_WINDOWS)

#include "DX12Shader.h"
#include "DX12Device.h"

#include <filesystem>
#include <winnt.h>

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "ntdll.lib")

namespace Atom
{
    namespace Utils
    {
        u32 GetShaderTypeSize(D3D12_SHADER_TYPE_DESC type)
        {
            switch (type.Type)
            {
                case D3D_SVT_UINT8:
                    return 1 * type.Columns * type.Rows;
                case D3D_SVT_INT16:
                case D3D_SVT_UINT16:
                case D3D_SVT_FLOAT16:
                    return 2 * type.Columns * type.Rows;
                case D3D_SVT_BOOL:
                case D3D_SVT_INT:
                case D3D_SVT_FLOAT:
                case D3D_SVT_UINT:
                    return 4 * type.Columns * type.Rows;
                case D3D_SVT_INT64:
                case D3D_SVT_UINT64:
                case D3D_SVT_DOUBLE:
                    return 8 * type.Columns * type.Rows;
            }

            ATOM_ENGINE_ASSERT(false, "Unsupported type!");
            return 0;
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    DX12Shader::DX12Shader(const String& filepath)
        : m_Name(std::filesystem::path(filepath).stem().string()), m_Filepath(filepath)
    {
        auto d3dDevice = Renderer::GetDevice().As<DX12Device>()->GetD3DDevice();

#if defined(ATOM_DEBUG)
        UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
        UINT compileFlags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

        // Read the file
        String source = ReadFile(filepath);
        ComPtr<ID3DBlob> errorBlob = nullptr;

        // Compile vertex shader
        D3DCompile(source.data(), source.size(), nullptr, nullptr, nullptr, "VSMain", "vs_5_1", compileFlags, 0, &m_VSData, &errorBlob);
        ATOM_ENGINE_ASSERT(!(errorBlob && errorBlob->GetBufferSize()), (char*)errorBlob->GetBufferPointer());

        // Compile pixel shader
        D3DCompile(source.data(), source.size(), nullptr, nullptr, nullptr, "PSMain", "ps_5_1", compileFlags, 0, &m_PSData, &errorBlob);
        ATOM_ENGINE_ASSERT(!(errorBlob && errorBlob->GetBufferSize()), (char*)errorBlob->GetBufferPointer());

        // Reflect on the shader data and build resource set
        ATOM_INFO("Shader \"{0}\" Resources:", m_Name);
        Reflect(m_VSData);
        Reflect(m_PSData);
        CreateRootSignature();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    DX12Shader::~DX12Shader()
    {
        
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const String& DX12Shader::GetName() const
    {
        return m_Name;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const String& DX12Shader::GetFilepath() const
    {
        return m_Filepath;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u64 DX12Shader::GetHash() const
    {
        return std::hash<String>{}(m_Filepath);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    String DX12Shader::ReadFile(const String& filepath)
    {
        std::ifstream fileStream(filepath, std::ios::binary | std::ios::in);

        fileStream.seekg(0, fileStream.end);
        u32 size = fileStream.tellg();
        fileStream.seekg(0, fileStream.beg);

        String buffer(size, '\0');

        if (size > 0)
        {
            fileStream.read(buffer.data(), size);
        }

        return buffer;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void DX12Shader::Reflect(const ComPtr<ID3DBlob>& shaderDataBlob)
    {
        ComPtr<ID3D12ShaderReflection> reflection = nullptr;
        DXCall(D3DReflect(shaderDataBlob->GetBufferPointer(), shaderDataBlob->GetBufferSize(), IID_PPV_ARGS(&reflection)));

        D3D12_SHADER_DESC shaderDesc = {};
        DXCall(reflection->GetDesc(&shaderDesc));

        for (u32 i = 0; i < shaderDesc.BoundResources; i++)
        {
            D3D12_SHADER_INPUT_BIND_DESC resourceDesc = {};
            DXCall(reflection->GetResourceBindingDesc(i, &resourceDesc));

            switch (resourceDesc.Type)
            {
                case D3D_SIT_CBUFFER:
                {
                    ATOM_INFO("\tConstant buffer \"{0}\" (b{1}, space({2}))", resourceDesc.Name, resourceDesc.BindPoint, resourceDesc.Space);

                    if (String(resourceDesc.Name)._Starts_with("mat_"))
                    {
                        // Add material constant buffer
                        ID3D12ShaderReflectionConstantBuffer* constantBuffer = reflection->GetConstantBufferByName(resourceDesc.Name);
                        D3D12_SHADER_BUFFER_DESC cbDesc = {};
                        constantBuffer->GetDesc(&cbDesc);

                        ConstantBuffer buffer;
                        buffer.Name = resourceDesc.Name;
                        buffer.Register = resourceDesc.BindPoint;
                        buffer.ShaderSpace = resourceDesc.Space;
                        buffer.Size = cbDesc.Size;

                        // Get the uniforms from the constant buffer struct type
                        ID3D12ShaderReflectionType* cbStructType = constantBuffer->GetVariableByIndex(0)->GetType();
                        D3D12_SHADER_TYPE_DESC cbStructTypeDesc = {};
                        cbStructType->GetDesc(&cbStructTypeDesc);

                        for (u32 i = 0; i < cbStructTypeDesc.Members; i++)
                        {
                            LPCSTR uniformName = cbStructType->GetMemberTypeName(i);
                            ID3D12ShaderReflectionType* uniformType = cbStructType->GetMemberTypeByIndex(i);
                            D3D12_SHADER_TYPE_DESC uniformTypeDesc = {};
                            uniformType->GetDesc(&uniformTypeDesc);

                            buffer.Uniforms.push_back({ uniformName, uniformTypeDesc.Type, uniformTypeDesc.Offset, Utils::GetShaderTypeSize(uniformTypeDesc) });
                        }

                        m_Constants[buffer.Name] = buffer;
                    }
                    else
                    {
                        ShaderResource resource = { resourceDesc.Name, resourceDesc.Type, resourceDesc.BindPoint, resourceDesc.Space };
                        m_DescriptorTable.ConstantBuffers[resource.Register] = resource;
                    }

                    break;
                }
                case D3D_SIT_STRUCTURED:
                case D3D_SIT_TEXTURE:
                {
                    ATOM_INFO("\tResourceRO \"{0}\" (s{1}, space({2}))", resourceDesc.Name, resourceDesc.BindPoint, resourceDesc.Space);

                    ShaderResource resource = { resourceDesc.Name, resourceDesc.Type, resourceDesc.BindPoint, resourceDesc.Space };
                    m_DescriptorTable.ResourcesRO[resource.Register] = resource;
                    m_Resources[resourceDesc.Name] = resource;

                    break;
                }
                case D3D_SIT_UAV_RWTYPED:
                case D3D_SIT_UAV_RWSTRUCTURED:
                {
                    ATOM_INFO("\tResourceRW \"{0}\" (s{1}, space({2}))", resourceDesc.Name, resourceDesc.BindPoint, resourceDesc.Space);

                    ShaderResource resource = { resourceDesc.Name, resourceDesc.Type, resourceDesc.BindPoint, resourceDesc.Space };
                    m_DescriptorTable.ResourcesRW[resource.Register] = resource;
                    m_Resources[resourceDesc.Name] = resource;

                    break;
                }
                case D3D_SIT_SAMPLER:
                {
                    ATOM_INFO("\tSampler \"{0}\" (s{1}, space({2}))", resourceDesc.Name, resourceDesc.BindPoint, resourceDesc.Space);

                    ShaderResource resource = { resourceDesc.Name, resourceDesc.Type, resourceDesc.BindPoint, resourceDesc.Space };
                    m_DescriptorTable.Samplers[resource.Register] = resource;
                    m_Resources[resourceDesc.Name] = resource;

                    break;
                }
                default:
                    ATOM_ENGINE_ASSERT(false, "Unsupported type!");
            }
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void DX12Shader::CreateRootSignature()
    {
        Vector<CD3DX12_ROOT_PARAMETER1> rootParameters;

        // Create root constants first
        for (auto& [name, constantBuffer] : m_Constants)
        {
            CD3DX12_ROOT_PARAMETER1 param;
            param.InitAsConstants(constantBuffer.Size / 4, constantBuffer.Register, constantBuffer.ShaderSpace, D3D12_SHADER_VISIBILITY_ALL);

            rootParameters.push_back(param);
        }

        // Create resource descriptor table
        Vector<CD3DX12_DESCRIPTOR_RANGE1> ranges;

        if (!m_DescriptorTable.ResourcesRO.empty())
        {
            CD3DX12_DESCRIPTOR_RANGE1 range;
            range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, m_DescriptorTable.ResourcesRO.size(), 0, 0);
            ranges.push_back(range);
        }
        if (!m_DescriptorTable.ConstantBuffers.empty())
        {
            CD3DX12_DESCRIPTOR_RANGE1 range;
            range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, m_DescriptorTable.ConstantBuffers.size(), 0, 0);
            ranges.push_back(range);
        }
        if (!m_DescriptorTable.ResourcesRW.empty())
        {
            CD3DX12_DESCRIPTOR_RANGE1 range;
            range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, m_DescriptorTable.ResourcesRW.size(), 0, 0);
            ranges.push_back(range);
        }

        if (ranges.size() > 0)
        {
            CD3DX12_ROOT_PARAMETER1 param;
            param.InitAsDescriptorTable(ranges.size(), ranges.data(), D3D12_SHADER_VISIBILITY_ALL);
            rootParameters.push_back(param);
        }

        // Create sampler descriptor table
        if (!m_DescriptorTable.Samplers.empty())
        {
            CD3DX12_DESCRIPTOR_RANGE1 range;
            range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, m_DescriptorTable.Samplers.size(), 0, 0);

            CD3DX12_ROOT_PARAMETER1 param;
            param.InitAsDescriptorTable(1, &range, D3D12_SHADER_VISIBILITY_ALL);
            rootParameters.push_back(param);
        }

        // Create root signature
        auto d3dDevice = Renderer::GetDevice().As<DX12Device>()->GetD3DDevice();

        D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

        if (FAILED(d3dDevice->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
        {
            featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
        }

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
        rootSignatureDesc.Init_1_1(rootParameters.size(), rootParameters.data(), 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
        
        ComPtr<ID3DBlob> signatureBlob = nullptr;
        ComPtr<ID3DBlob> errorBlob = nullptr;

        D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signatureBlob, &errorBlob);
        ATOM_ENGINE_ASSERT(!(errorBlob && errorBlob->GetBufferSize()), (char*)errorBlob->GetBufferPointer());

        DXCall(d3dDevice->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature)));
    }
}

#endif // ATOM_PLATFORM_WINDOWS
