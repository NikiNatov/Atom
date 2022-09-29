#include "atompch.h"

#include "Shader.h"
#include "Device.h"
#include "Atom/Core/DirectX12/DirectX12Utils.h"

#include <filesystem>
#include <winnt.h>

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "ntdll.lib")

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    ShaderResourceLayout::ShaderResourceLayout(const ComPtr<ID3DBlob>& vsDataBlob, const ComPtr<ID3DBlob>& psDataBlob)
    {
        Reflect(vsDataBlob);
        Reflect(psDataBlob);
        CreateRootSignature();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    ShaderResourceLayout::~ShaderResourceLayout()
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ShaderResourceLayout::Reflect(const ComPtr<ID3DBlob>& shaderDataBlob)
    {
        ComPtr<ID3D12ShaderReflection> reflection = nullptr;
        DXCall(D3DReflect(shaderDataBlob->GetBufferPointer(), shaderDataBlob->GetBufferSize(), IID_PPV_ARGS(&reflection)));

        D3D12_SHADER_DESC shaderDesc = {};
        DXCall(reflection->GetDesc(&shaderDesc));

        for (u32 i = 0; i < shaderDesc.BoundResources; i++)
        {
            D3D12_SHADER_INPUT_BIND_DESC resourceDesc = {};
            DXCall(reflection->GetResourceBindingDesc(i, &resourceDesc));

            ShaderResourceType type = Utils::D3D12ShaderResourceTypeToAtom(resourceDesc);

            switch (type)
            {
                case ShaderResourceType::ConstantBuffer:
                {
                    ATOM_ENGINE_INFO("\tConstantBuffer \"{0}\" (b{1}, space({2}))", resourceDesc.Name, resourceDesc.BindPoint, resourceDesc.Space);

                    ID3D12ShaderReflectionConstantBuffer* constantBuffer = reflection->GetConstantBufferByName(resourceDesc.Name);
                    D3D12_SHADER_BUFFER_DESC cbDesc = {};
                    constantBuffer->GetDesc(&cbDesc);

                    if (cbDesc.Variables > 1)
                    {
                        // Add constant buffer as root constant
                        ConstantBuffer buffer;
                        buffer.Name = resourceDesc.Name;
                        buffer.Register = resourceDesc.BindPoint;
                        buffer.ShaderSpace = resourceDesc.Space;
                        buffer.Size = cbDesc.Size;

                        // Get the uniforms from the constant buffer struct type
                        for (u32 i = 0; i < cbDesc.Variables; i++)
                        {
                            ID3D12ShaderReflectionVariable* shaderUniform = constantBuffer->GetVariableByIndex(i);
                            D3D12_SHADER_VARIABLE_DESC shaderUniformDesc = {};
                            shaderUniform->GetDesc(&shaderUniformDesc);

                            // Get type information about the uniform
                            ID3D12ShaderReflectionType* shaderUniformType = shaderUniform->GetType();
                            D3D12_SHADER_TYPE_DESC shaderUniformTypeDesc = {};
                            shaderUniformType->GetDesc(&shaderUniformTypeDesc);

                            ShaderDataType uniformType = Utils::D3D12ShaderTypeToAtom(shaderUniformTypeDesc);
                            ATOM_ENGINE_ASSERT(uniformType != ShaderDataType::None, fmt::format("Shader uniforms with type \"{0}\" are not supported!", shaderUniformTypeDesc.Name));
                            buffer.Uniforms.push_back({ shaderUniformDesc.Name, uniformType, shaderUniformDesc.StartOffset, shaderUniformDesc.Size, buffer.Register });
                        }

                        m_RootConstants.push_back(buffer);
                    }
                    else
                    {
                        // Add constant buffer as shader resource
                        m_Resources.emplace_back(resourceDesc.Name, type, resourceDesc.BindPoint, resourceDesc.Space);
                    }

                    break;
                }
                case ShaderResourceType::StructuredBuffer:
                case ShaderResourceType::Texture2D:
                case ShaderResourceType::Texture2DArray:
                case ShaderResourceType::TextureCube:
                {
                    ATOM_ENGINE_INFO("\ShaderResourceRO \"{0}\" (t{1}, space({2}))", resourceDesc.Name, resourceDesc.BindPoint, resourceDesc.Space);
                    m_Resources.emplace_back(resourceDesc.Name, type, resourceDesc.BindPoint, resourceDesc.Space);
                    break;
                }
                case ShaderResourceType::RWStructuredBuffer:
                case ShaderResourceType::RWTexture2D:
                case ShaderResourceType::RWTexture2DArray:
                {
                    ATOM_ENGINE_INFO("\ShaderResourceRW \"{0}\" (u{1}, space({2}))", resourceDesc.Name, resourceDesc.BindPoint, resourceDesc.Space);
                    m_Resources.emplace_back(resourceDesc.Name, type, resourceDesc.BindPoint, resourceDesc.Space);
                    break;
                }
                case ShaderResourceType::Sampler:
                {
                    ATOM_ENGINE_INFO("\tSampler \"{0}\" (s{1}, space({2}))", resourceDesc.Name, resourceDesc.BindPoint, resourceDesc.Space);
                    m_Resources.emplace_back(resourceDesc.Name, type, resourceDesc.BindPoint, resourceDesc.Space);
                    break;
                }
                default:
                    ATOM_ENGINE_ASSERT(false, "Unsupported type!");
            }
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ShaderResourceLayout::CreateRootSignature()
    {
        // Sort resources
        Vector<ShaderResource*> inlineResources;
        Vector<ShaderResource*> readOnlyResources;
        Vector<ShaderResource*> readWriteResources;
        Vector<ShaderResource*> samplers;

        for (auto& resource : m_Resources)
        {
            if (resource.Type == ShaderResourceType::ConstantBuffer || resource.Type == ShaderResourceType::StructuredBuffer)
            {
                inlineResources.push_back(&resource);
            }
            else if (resource.Type == ShaderResourceType::Sampler)
            {
                samplers.push_back(&resource);
            }
            else
            {
                if (resource.IsReadOnly())
                    readOnlyResources.push_back(&resource);
                else
                    readWriteResources.push_back(&resource);
            }
        }

        std::sort(readOnlyResources.begin(), readOnlyResources.end(), [](const ShaderResource* lhs, const ShaderResource* rhs) { return lhs->Register < rhs->Register; });
        std::sort(readWriteResources.begin(), readWriteResources.end(), [](const ShaderResource* lhs, const ShaderResource* rhs) { return lhs->Register < rhs->Register; });
        std::sort(samplers.begin(), samplers.end(), [](const ShaderResource* lhs, const ShaderResource* rhs) { return lhs->Register < rhs->Register; });

        // Create root constants first
        Vector<CD3DX12_ROOT_PARAMETER1> rootParameters;

        for (auto& constantBuffer : m_RootConstants)
        {
            CD3DX12_ROOT_PARAMETER1 param;
            param.InitAsConstants(constantBuffer.Size / 4, constantBuffer.Register, constantBuffer.ShaderSpace, D3D12_SHADER_VISIBILITY_ALL);
            rootParameters.push_back(param);
        }

        // Create root descriptors for constant buffers
        for (auto resource : inlineResources)
        {
            CD3DX12_ROOT_PARAMETER1 param;

            switch (resource->Type)
            {
                case ShaderResourceType::ConstantBuffer:
                {
                    param.InitAsConstantBufferView(resource->Register, resource->ShaderSpace, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL);
                    m_InlineDescriptors.emplace_back(rootParameters.size(), ShaderInlineDescriptorType::CBV);
                    break;
                }
                case ShaderResourceType::StructuredBuffer:
                {
                    param.InitAsShaderResourceView(resource->Register, resource->ShaderSpace, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL);
                    m_InlineDescriptors.emplace_back(rootParameters.size(), ShaderInlineDescriptorType::SRV);
                    break;
                }
                case ShaderResourceType::RWStructuredBuffer:
                {
                    param.InitAsUnorderedAccessView(resource->Register, resource->ShaderSpace, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL);
                    m_InlineDescriptors.emplace_back(rootParameters.size(), ShaderInlineDescriptorType::UAV);
                    break;
                }
                default:
                    ATOM_ENGINE_ASSERT(false, "Shader resource type is not supported to be used as inline descriptor");
            }
            
            rootParameters.push_back(param);
        }

        // Create resource descriptor table
        Vector<CD3DX12_DESCRIPTOR_RANGE1> ranges;

        if (!readOnlyResources.empty())
        {
            ranges.emplace_back(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, readOnlyResources.size(), readOnlyResources[0]->Register, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);
        }
        if (!readWriteResources.empty())
        {
            ranges.emplace_back(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, readWriteResources.size(), readWriteResources[0]->Register, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);
        }
        if (ranges.size() > 0)
        {
            CD3DX12_ROOT_PARAMETER1 param;
            param.InitAsDescriptorTable(ranges.size(), ranges.data(), D3D12_SHADER_VISIBILITY_ALL);
            m_DescriptorTables.emplace_back(rootParameters.size(), DescriptorHeapType::ShaderResource, readOnlyResources.size() + readWriteResources.size());
            rootParameters.push_back(param);
        }

        // Create sampler descriptor table
        if (!samplers.empty())
        {
            CD3DX12_DESCRIPTOR_RANGE1 range(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, samplers.size(), samplers[0]->Register, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);
            CD3DX12_ROOT_PARAMETER1 param;
            param.InitAsDescriptorTable(1, &range, D3D12_SHADER_VISIBILITY_ALL);
            m_DescriptorTables.emplace_back(rootParameters.size(), DescriptorHeapType::Sampler, samplers.size());
            rootParameters.push_back(param);
        }

        // Create root signature
        auto d3dDevice = Device::Get().GetD3DDevice();

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

    // -----------------------------------------------------------------------------------------------------------------------------
    Shader::Shader(const String& filepath)
        : m_Name(std::filesystem::path(filepath).stem().string()), m_Filepath(filepath)
    {
        auto d3dDevice = Device::Get().GetD3DDevice();

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
        ATOM_ENGINE_INFO("Shader \"{0}\" Resources:", m_Name);
        m_ResourceLayout = ShaderResourceLayout(m_VSData, m_PSData);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Shader::~Shader()
    {

    }

    // -----------------------------------------------------------------------------------------------------------------------------
    String Shader::ReadFile(const String& filepath)
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
}
