#include "atompch.h"

#include "Shader.h"
#include "Atom/Core/DirectX12/DirectX12Utils.h"
#include "Atom/Renderer/Device.h"

#include <filesystem>

#pragma comment(lib, "d3dcompiler.lib")

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    Shader::Shader(const std::filesystem::path& filepath)
        : m_Name(filepath.stem().string()), m_Filepath(filepath)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    GraphicsShader::GraphicsShader(const std::filesystem::path& filepath)
        : Shader(filepath)
    {
        auto d3dDevice = Device::Get().GetD3DDevice();

#if defined(ATOM_DEBUG)
        UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
        UINT compileFlags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif
        ComPtr<ID3DBlob> errorBlob = nullptr;
        WString filepathStr = filepath.wstring();

        // Compile vertex shader
        D3DCompileFromFile(filepathStr.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_1", compileFlags, 0, &m_VSData, &errorBlob);
        ATOM_ENGINE_ASSERT(!(errorBlob && errorBlob->GetBufferSize()), (char*)errorBlob->GetBufferPointer());
        
        // Compile pixel shader
        D3DCompileFromFile(filepathStr.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_1", compileFlags, 0, &m_PSData, &errorBlob);
        ATOM_ENGINE_ASSERT(!(errorBlob && errorBlob->GetBufferSize()), (char*)errorBlob->GetBufferPointer());

        // Reflect on the shader data and build resource set
        ATOM_ENGINE_INFO("Shader \"{0}\" Resources:", m_Name);
        m_ShaderLayout = ShaderLayout(m_VSData, m_PSData);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    GraphicsShader::~GraphicsShader()
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    ComputeShader::ComputeShader(const std::filesystem::path& filepath)
        : Shader(filepath)
    {
        auto d3dDevice = Device::Get().GetD3DDevice();

#if defined(ATOM_DEBUG)
        UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
        UINT compileFlags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif
        ComPtr<ID3DBlob> errorBlob = nullptr;

        // Compile compute shader
        D3DCompileFromFile(filepath.wstring().c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "CSMain", "cs_5_1", compileFlags, 0, &m_CSData, &errorBlob);
        ATOM_ENGINE_ASSERT(!(errorBlob && errorBlob->GetBufferSize()), (char*)errorBlob->GetBufferPointer());

        // Reflect on the shader data and build resource set
        ATOM_ENGINE_INFO("Compute Shader \"{0}\" Resources:", m_Name);
        m_ShaderLayout = ShaderLayout(m_CSData);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    ComputeShader::~ComputeShader()
    {
    }
}
