#include "atompch.h"
#include "Shader.h"

#include "Atom/Core/Hash.h"
#include "Atom/Core/DirectX12/DirectX12Utils.h"
#include "Atom/Renderer/Device.h"

#include <filesystem>

#pragma comment(lib, "d3dcompiler.lib")

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    Shader::Shader(const String& name, const Vector<byte>& vsData, const Vector<byte>& psData)
        : m_Name(name), m_ShaderType(ShaderType::Graphics), m_ShaderReflection(vsData, psData)
    {
        HashBuilder hashBuilder;

        for (byte b : vsData)
            hashBuilder.AddToHash(b);
        for (byte b : psData)
            hashBuilder.AddToHash(b);

        m_Hash = hashBuilder.GetHash();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Shader::Shader(const String& name, const Vector<byte>& csData)
        : m_Name(name), m_ShaderType(ShaderType::Compute), m_ShaderReflection(csData)
    {
        HashBuilder hashBuilder;

        for (byte b : csData)
            hashBuilder.AddToHash(b);

        m_Hash = hashBuilder.GetHash();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    GraphicsShader::GraphicsShader(const String& name, const Vector<byte>& vsData, const Vector<byte>& psData)
        : Shader(name, vsData, psData), m_VSData(vsData), m_PSData(psData)
    {
        m_D3DVSByteCode = { m_VSData.data(), m_VSData.size() };
        m_D3DPSByteCode = { m_PSData.data(), m_PSData.size() };
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    GraphicsShader::~GraphicsShader()
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    ComputeShader::ComputeShader(const String& name, const Vector<byte>& csData)
        : Shader(name, csData), m_CSData(csData)
    {
        m_D3DCSByteCode = { m_CSData.data(), m_CSData.size() };
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    ComputeShader::~ComputeShader()
    {
    }
}
