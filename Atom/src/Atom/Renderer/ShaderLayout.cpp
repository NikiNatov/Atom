#include "atompch.h"

#include "Shader.h"
#include "Device.h"
#include "Atom/Core/DirectX12/DirectX12Utils.h"

namespace Atom
{
    namespace Utils
    {
        const char* ShaderBindPointToString(ShaderBindPoint bindPoint)
        {
            switch (bindPoint)
            {
                case ShaderBindPoint::Instance: return "Instance";
                case ShaderBindPoint::Material: return "Material";
                case ShaderBindPoint::Pass:     return "Pass";
                case ShaderBindPoint::Frame:    return "Frame";
            }

            return nullptr;
        }

        const char* ShaderDataTypeToString(ShaderDataType type)
        {
            switch (type)
            {
                case ShaderDataType::Unorm4: return "Unorm4";
                case ShaderDataType::Int:    return "Int";
                case ShaderDataType::Int2:   return "Int2";
                case ShaderDataType::Int3:   return "Int3";
                case ShaderDataType::Int4:   return "Int4";
                case ShaderDataType::Uint:   return "Uint";
                case ShaderDataType::Uint2:  return "Uint2";
                case ShaderDataType::Uint3:  return "Uint3";
                case ShaderDataType::Uint4:  return "Uint4";
                case ShaderDataType::Float:  return "Float";
                case ShaderDataType::Float2: return "Float2";
                case ShaderDataType::Float3: return "Float3";
                case ShaderDataType::Float4: return "Float4";
                case ShaderDataType::Bool:   return "Bool";
                case ShaderDataType::Mat2:   return "Mat2";
                case ShaderDataType::Mat3:   return "Mat3";
                case ShaderDataType::Mat4:   return "Mat4";
            }

            return nullptr;
        }

        const char* ShaderResourceTypeToString(ShaderResourceType type)
        {
            switch (type)
            {
                case ShaderResourceType::ConstantBuffer:     return "ConstantBuffer";
                case ShaderResourceType::Texture2D:          return "Texture2D";
                case ShaderResourceType::Texture2DArray:     return "Texture2DArray";
                case ShaderResourceType::TextureCube:        return "TextureCube";
                case ShaderResourceType::RWTexture2D:        return "RWTexture2D";
                case ShaderResourceType::RWTexture2DArray:   return "RWTexture2DArray";
                case ShaderResourceType::StructuredBuffer:   return "StructuredBuffer";
                case ShaderResourceType::RWStructuredBuffer: return "RWStructuredBuffer";
                case ShaderResourceType::Sampler:            return "Sampler";
            }

            return nullptr;
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    ShaderLayout::ShaderLayout()
        : m_RootSignature(nullptr)
    {
        for (u32 i = 0; i < u32(ShaderBindPoint::NumBindPoints); i++)
        {
            ShaderBindPoint bindPoint = (ShaderBindPoint)i;
            m_Constants[i].BindPoint = bindPoint;
            m_ResourceDescriptorTables[i].BindPoint = bindPoint;
            m_SamplerDescriptorTables[i].BindPoint = bindPoint;
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    ShaderLayout::ShaderLayout(const ComPtr<ID3DBlob>& vsDataBlob, const ComPtr<ID3DBlob>& psDataBlob)
        : ShaderLayout()
    {
        Reflect(vsDataBlob);
        Reflect(psDataBlob);
        LogReflectionInfo();
        CreateRootSignature();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    ShaderLayout::ShaderLayout(const ComPtr<ID3DBlob>& csDataBlob)
        : ShaderLayout()
    {
        Reflect(csDataBlob);
        LogReflectionInfo();
        CreateRootSignature();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    ShaderLayout::~ShaderLayout()
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ShaderLayout::Reflect(const ComPtr<ID3DBlob>& shaderDataBlob)
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
            ShaderBindPoint bindPoint = (ShaderBindPoint)resourceDesc.Space;

            switch (type)
            {
                case ShaderResourceType::ConstantBuffer:
                {
                    ID3D12ShaderReflectionConstantBuffer* constantBuffer = reflection->GetConstantBufferByName(resourceDesc.Name);
                    D3D12_SHADER_BUFFER_DESC cbDesc = {};
                    constantBuffer->GetDesc(&cbDesc);

                    m_Constants[(u32)bindPoint].Register = resourceDesc.BindPoint;
                    m_Constants[(u32)bindPoint].TotalSize = cbDesc.Size;

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
                        String uniformName = shaderUniformDesc.Name;
                        ATOM_ENGINE_ASSERT(uniformType != ShaderDataType::None, fmt::format("Shader uniforms with type \"{0}\" are not supported!", shaderUniformTypeDesc.Name));
                        m_Constants[(u32)bindPoint].Uniforms.emplace_back(uniformName.substr(uniformName.find('_') + 1), uniformType, 
                            resourceDesc.BindPoint, shaderUniformDesc.StartOffset, shaderUniformDesc.Size);
                    }

                    break;
                }
                case ShaderResourceType::StructuredBuffer:
                case ShaderResourceType::Texture2D:
                case ShaderResourceType::Texture2DArray:
                case ShaderResourceType::TextureCube:
                case ShaderResourceType::RWStructuredBuffer:
                case ShaderResourceType::RWTexture2D:
                case ShaderResourceType::RWTexture2DArray:
                {
                    String resourceName = resourceDesc.Name;
                    m_ResourceDescriptorTables[(u32)bindPoint].Resources.emplace_back(resourceName.substr(resourceName.find('_') + 1), type, resourceDesc.BindPoint);
                    break;
                }
                case ShaderResourceType::Sampler:
                {
                    String resourceName = resourceDesc.Name;
                    m_SamplerDescriptorTables[(u32)bindPoint].Resources.emplace_back(resourceName.substr(resourceName.find('_') + 1), type, resourceDesc.BindPoint);
                    break;
                }
                default:
                    ATOM_ENGINE_ASSERT(false, "Unsupported type!");
            }
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ShaderLayout::CreateRootSignature()
    {
        struct DescriptorRangeEntry
        {
            Vector<CD3DX12_DESCRIPTOR_RANGE1> ResourceRanges;
            CD3DX12_DESCRIPTOR_RANGE1 SamplerRange;
        };

        DescriptorRangeEntry ranges[u32(ShaderBindPoint::NumBindPoints)];

        Vector<CD3DX12_ROOT_PARAMETER1> rootParameters;

        for (u32 bindPoint = 0; bindPoint < u32(ShaderBindPoint::NumBindPoints); bindPoint++)
        {
            ShaderConstants& constants = m_Constants[bindPoint];
            ShaderDescriptorTable& resourceTable = m_ResourceDescriptorTables[bindPoint];
            ShaderDescriptorTable& samplerTable = m_SamplerDescriptorTables[bindPoint];

            if (!constants.Uniforms.empty())
            {
                CD3DX12_ROOT_PARAMETER1 constantsParam;
                if (bindPoint == u32(ShaderBindPoint::Instance) || bindPoint == u32(ShaderBindPoint::Material))
                {
                    constantsParam.InitAsConstants(constants.TotalSize / 4, constants.Register, bindPoint, D3D12_SHADER_VISIBILITY_ALL);
                }
                else
                {
                    constantsParam.InitAsConstantBufferView(constants.Register, bindPoint, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL);
                }

                constants.RootParameterIndex = rootParameters.size();
                rootParameters.push_back(constantsParam);
            }

            if (!resourceTable.Resources.empty())
            {
                Vector<ShaderResource*> readOnlyResources;
                Vector<ShaderResource*> readWriteResources;

                for (auto& resource : resourceTable.Resources)
                {
                    switch (resource.Type)
                    {
                        case ShaderResourceType::StructuredBuffer:
                        case ShaderResourceType::Texture2D:
                        case ShaderResourceType::Texture2DArray:
                        case ShaderResourceType::TextureCube:
                        {
                            readOnlyResources.push_back(&resource);
                            break;
                        }
                        case ShaderResourceType::RWStructuredBuffer:
                        case ShaderResourceType::RWTexture2D:
                        case ShaderResourceType::RWTexture2DArray:
                        {
                            readWriteResources.push_back(&resource);
                            break;
                        }
                    }
                }

                if (!readOnlyResources.empty())
                {
                    ranges[bindPoint].ResourceRanges.emplace_back(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, readOnlyResources.size(), 0, bindPoint, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);
                }
                if (!readWriteResources.empty())
                {
                    ranges[bindPoint].ResourceRanges.emplace_back(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, readWriteResources.size(), 0, bindPoint, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);
                }

                CD3DX12_ROOT_PARAMETER1 resourceDescriptorTableParam;
                resourceDescriptorTableParam.InitAsDescriptorTable(ranges[bindPoint].ResourceRanges.size(), ranges[bindPoint].ResourceRanges.data(), D3D12_SHADER_VISIBILITY_ALL);
                resourceTable.RootParameterIndex = rootParameters.size();
                rootParameters.push_back(resourceDescriptorTableParam);
            }
            
            if (!samplerTable.Resources.empty())
            {
                ranges[bindPoint].SamplerRange = { D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, (u32)samplerTable.Resources.size(), 0, bindPoint, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE };

                CD3DX12_ROOT_PARAMETER1 samplerDescriptorTableParam;
                samplerDescriptorTableParam.InitAsDescriptorTable(1, &ranges[bindPoint].SamplerRange, D3D12_SHADER_VISIBILITY_ALL);
                samplerTable.RootParameterIndex = rootParameters.size();
                rootParameters.push_back(samplerDescriptorTableParam);
            }
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
    void ShaderLayout::LogReflectionInfo()
    {
        // Print debug info
        for (u32 bindPoint = 0; bindPoint < u32(ShaderBindPoint::NumBindPoints); bindPoint++)
        {
            const ShaderConstants& constants = m_Constants[bindPoint];
            const ShaderDescriptorTable& resourceTable = m_ResourceDescriptorTables[bindPoint];
            const ShaderDescriptorTable& samplerTable = m_SamplerDescriptorTables[bindPoint];

            if (!constants.Uniforms.empty())
            {
                ATOM_ENGINE_INFO("\tConstants BindPoint={0}", Utils::ShaderBindPointToString(constants.BindPoint));
                for (const auto& uniform : constants.Uniforms)
                {
                    ATOM_ENGINE_INFO("\t\t{0} {1}", Utils::ShaderDataTypeToString(uniform.Type), uniform.Name);
                }
            }

            if (!resourceTable.Resources.empty())
            {
                ATOM_ENGINE_INFO("\tResourceTable BindPoint={0}", Utils::ShaderBindPointToString(resourceTable.BindPoint));
                for (const auto& resource : resourceTable.Resources)
                {
                    ATOM_ENGINE_INFO("\t\t{0} {1}: register{2}", Utils::ShaderResourceTypeToString(resource.Type), resource.Name, resource.Register);
                }
            }

            if (!samplerTable.Resources.empty())
            {
                ATOM_ENGINE_INFO("\tSamplerTable BindPoint={0}", Utils::ShaderBindPointToString(samplerTable.BindPoint));
                for (const auto& resource : samplerTable.Resources)
                {
                    ATOM_ENGINE_INFO("\t\t{0} {1}: register{2}", Utils::ShaderResourceTypeToString(resource.Type), resource.Name, resource.Register);
                }
            }
        }
    }
}