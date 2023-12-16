#include "atompch.h"
#include "ShaderReflection.h"

#include "Atom/Core/DirectX12/DirectX12Utils.h"

#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    ShaderReflection::ShaderReflection(const Vector<byte>& vsData, const Vector<byte>& psData)
    {
        Reflect(vsData);
        Reflect(psData);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    ShaderReflection::ShaderReflection(const Vector<byte>& csData)
    {
        Reflect(csData);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ShaderReflection::Reflect(const Vector<byte>& shaderDataBlob)
    {
        ComPtr<ID3D12ShaderReflection> reflection = nullptr;
        DXCall(D3DReflect(shaderDataBlob.data(), shaderDataBlob.size(), IID_PPV_ARGS(&reflection)));

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
                    ID3D12ShaderReflectionConstantBuffer* constantBuffer = reflection->GetConstantBufferByName(resourceDesc.Name);
                    D3D12_SHADER_BUFFER_DESC cbDesc = {};
                    constantBuffer->GetDesc(&cbDesc);

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

                        m_Constants[resourceDesc.Space].emplace_back(uniformName.substr(uniformName.find('_') + 1), uniformType, resourceDesc.BindPoint, shaderUniformDesc.StartOffset, shaderUniformDesc.Size);
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
                    m_Resources[resourceDesc.Space].emplace_back(resourceName.substr(resourceName.find('_') + 1), type, resourceDesc.BindPoint);
                    break;
                }
                case ShaderResourceType::Sampler:
                {
                    String samplerName = resourceDesc.Name;
                    m_Samplers[resourceDesc.Space].emplace_back(samplerName.substr(samplerName.find('_') + 1), type, resourceDesc.BindPoint);
                    break;
                }
                default:
                    ATOM_ENGINE_ASSERT(false, "Unsupported type!");
            }
        }
    }
}