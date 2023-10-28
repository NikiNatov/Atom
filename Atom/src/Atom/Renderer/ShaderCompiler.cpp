#include "atompch.h"
#include "ShaderCompiler.h"

#include "Atom/Core/DirectX12/DirectX12.h"

#include <d3dcompiler.h>

#pragma comment(lib, "d3dcompiler.lib")

namespace Atom
{
    static constexpr u32 VertexShaderIdx = 0;
    static constexpr u32 PixelShaderIdx = 1;
    static constexpr u32 ComputeShaderIdx = 2;

    static constexpr char* s_ShaderTypeTokens[] =
    {
        "#shadertype vs",
        "#shadertype ps",
        "#shadertype cs"
    };

    static constexpr char* s_ShaderEntryPoints[] =
    {
        "VSMain",
        "PSMain",
        "CSMain"
    };

    static constexpr char* s_ShaderTargets[] =
    {
        "vs_5_1",
        "ps_5_1",
        "cs_5_1"
    };

    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<GraphicsShader> ShaderCompiler::CompileGraphicsShader(const std::filesystem::path& filepath)
    {
        if (!std::filesystem::exists(ms_OutputDirectory))
            std::filesystem::create_directories(ms_OutputDirectory);

        Map<u32, std::ostringstream> shaderSources = PreProcessHlslFile(filepath);

        String shaderName = filepath.stem().string();

        Vector<byte> vsCompiledData;
        String vsSource = shaderSources[VertexShaderIdx].str();
        u64 vsSourceHash = std::hash<String>{}(vsSource);
        bool vsChanged = true;

        Vector<byte> psCompiledData;
        String psSource = shaderSources[PixelShaderIdx].str();
        u64 psSourceHash = std::hash<String>{}(psSource);
        bool psChanged = true;

        if (vsSource.empty())
        {
            ATOM_ENGINE_ERROR("ShaderCompiler: Shader {} is missing vertex shader code. Every graphics shader must at least a vertex shader", filepath.string().c_str());
            return nullptr;
        }

        // Check if we already have compiled binary for the shader
        std::filesystem::path shaderBinaryFilepath = ms_OutputDirectory / fmt::format("{}.gfxshader", shaderName);
        if (std::filesystem::exists(shaderBinaryFilepath))
        {
            std::ifstream ifs(shaderBinaryFilepath, std::ios::in | std::ios::binary);

            // Read shader header
            GraphicsShaderFileHeader shaderFileHeader;
            ifs.read((char*)&shaderFileHeader, sizeof(GraphicsShaderFileHeader));

            vsChanged = shaderFileHeader.VertexShaderHash != vsSourceHash;
            psChanged = shaderFileHeader.PixelShaderHash != psSourceHash;

            if (!vsChanged)
            {
                // Read cached vertex shader data
                vsCompiledData.resize(shaderFileHeader.VertexShaderDataSize);
                ifs.read((char*)vsCompiledData.data(), vsCompiledData.size());
            }

            if (!psChanged && shaderFileHeader.PixelShaderDataSize > 0)
            {
                // Read cached pixel shader data
                psCompiledData.resize(shaderFileHeader.PixelShaderDataSize);
                ifs.read((char*)psCompiledData.data(), psCompiledData.size());
            }

            if(!vsChanged && !psChanged)
                return CreateRef<GraphicsShader>(shaderName, vsCompiledData, psCompiledData);
        }

        // Changes in the shader where found so recompile it
        ATOM_ENGINE_INFO("ShaderCompiler: Compiling shader {}...", shaderBinaryFilepath.string());

        if (vsChanged)
            if (!Compile(vsSource, filepath, s_ShaderEntryPoints[VertexShaderIdx], s_ShaderTargets[VertexShaderIdx], vsCompiledData))
                return nullptr;

        if (psChanged && !psSource.empty())
            if (!Compile(psSource, filepath, s_ShaderEntryPoints[PixelShaderIdx], s_ShaderTargets[PixelShaderIdx], psCompiledData))
                return nullptr;

        // Create file on disk
        std::ofstream ofs(shaderBinaryFilepath, std::ios::out | std::ios::trunc | std::ios::binary);

        if (!ofs.is_open())
        {
            ATOM_ENGINE_ERROR("ShaderCompiler: Shader binary file {} could not be created", shaderBinaryFilepath.string());
            return nullptr;
        }

        GraphicsShaderFileHeader shaderFileHeader;
        shaderFileHeader.VertexShaderHash = vsSourceHash;
        shaderFileHeader.VertexShaderDataSize = vsCompiledData.size();
        shaderFileHeader.PixelShaderHash = psSourceHash;
        shaderFileHeader.PixelShaderDataSize = psCompiledData.size();

        ofs.write((char*)&shaderFileHeader, sizeof(GraphicsShaderFileHeader));
        ofs.write((char*)vsCompiledData.data(), vsCompiledData.size());

        if(!psCompiledData.empty())
            ofs.write((char*)psCompiledData.data(), psCompiledData.size());

        return CreateRef<GraphicsShader>(shaderName, vsCompiledData, psCompiledData);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<ComputeShader> ShaderCompiler::CompileComputeShader(const std::filesystem::path& filepath)
    {
        if (!std::filesystem::exists(ms_OutputDirectory))
            std::filesystem::create_directories(ms_OutputDirectory);

        Map<u32, std::ostringstream> shaderSources = PreProcessHlslFile(filepath);

        String shaderName = filepath.stem().string();

        Vector<byte> csCompiledData;
        String csSource = shaderSources[ComputeShaderIdx].str();
        u64 csSourceHash = std::hash<String>{}(csSource);

        if (csSource.empty())
        {
            ATOM_ENGINE_ERROR("ShaderCompiler: Shader {} is missing compute shader code.", filepath.string().c_str());
            return nullptr;
        }

        // Check if we already have compiled binary for the shader
        std::filesystem::path shaderBinaryFilepath = ms_OutputDirectory / fmt::format("{}.cshader", shaderName);
        if (std::filesystem::exists(shaderBinaryFilepath))
        {
            std::ifstream ifs(shaderBinaryFilepath, std::ios::in | std::ios::binary);

            // Read shader header
            ComputeShaderFileHeader shaderFileHeader;
            ifs.read((char*)&shaderFileHeader, sizeof(ComputeShaderFileHeader));

            if (shaderFileHeader.ShaderHash == csSourceHash)
            {
                // Read cached compute shader data
                csCompiledData.resize(shaderFileHeader.ShaderDataSize);
                ifs.read((char*)csCompiledData.data(), csCompiledData.size());

                return CreateRef<ComputeShader>(shaderName, csCompiledData);
            }
        }

        // Changes in the shader where found so recompile it
        ATOM_ENGINE_INFO("ShaderCompiler: Compiling shader {}...", shaderBinaryFilepath.string());

        if (!Compile(csSource, filepath, s_ShaderEntryPoints[ComputeShaderIdx], s_ShaderTargets[ComputeShaderIdx], csCompiledData))
            return nullptr;

        // Create file on disk
        std::ofstream ofs(shaderBinaryFilepath, std::ios::out | std::ios::trunc | std::ios::binary);

        if (!ofs.is_open())
        {
            ATOM_ENGINE_ERROR("ShaderCompiler: Shader binary file {} could not be created", shaderBinaryFilepath.string());
            return nullptr;
        }

        ComputeShaderFileHeader shaderFileHeader;
        shaderFileHeader.ShaderHash = csSourceHash;
        shaderFileHeader.ShaderDataSize = csCompiledData.size();

        ofs.write((char*)&shaderFileHeader, sizeof(ComputeShaderFileHeader));
        ofs.write((char*)csCompiledData.data(), csCompiledData.size());

        return CreateRef<ComputeShader>(shaderName, csCompiledData);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Map<u32, std::ostringstream> ShaderCompiler::PreProcessHlslFile(const std::filesystem::path& filepath)
    {
        if (!std::filesystem::exists(filepath))
        {
            ATOM_ENGINE_ERROR("ShaderCompiler: Shader file {} does not exist", filepath.string());
            return {};
        }

        std::ifstream ifs(filepath, std::ios::in);

        if (!ifs.is_open())
        {
            ATOM_ENGINE_ERROR("ShaderCompiler: Shader file {} could not be opened", filepath.string());
            return {};
        }

        // Extract shader sources
        Map<u32, std::ostringstream> shaderSources;
        u32 currentShaderIdx = UINT32_MAX;

        String line;
        while (std::getline(ifs, line))
        {
            if (line == s_ShaderTypeTokens[VertexShaderIdx])
            {
                currentShaderIdx = VertexShaderIdx;
            }
            else if (line == s_ShaderTypeTokens[PixelShaderIdx])
            {
                currentShaderIdx = PixelShaderIdx;
            }
            else if (line == s_ShaderTypeTokens[ComputeShaderIdx])
            {
                currentShaderIdx = ComputeShaderIdx;
            }
            else
            {
                if (currentShaderIdx == UINT32_MAX)
                {
                    ATOM_ENGINE_ERROR("ShaderCompiler: No shader type specified");
                    return {};
                }

                shaderSources[currentShaderIdx] << line << "\n";
            }
        }

        return shaderSources;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool ShaderCompiler::Compile(const String& shaderSource, const std::filesystem::path& shaderFilepath, const char* entryPoint, const char* target, Vector<byte>& compiledData)
    {
        u32 compileFlags = ms_OptimizeShaders ? D3DCOMPILE_OPTIMIZATION_LEVEL3 : (D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION);

        ComPtr<ID3DBlob> shaderBlob = nullptr;
        ComPtr<ID3DBlob> errorBlob = nullptr;

        D3DCompile(shaderSource.c_str(), shaderSource.length(), shaderFilepath.string().c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
            entryPoint, target, compileFlags, 0, &shaderBlob, &errorBlob);

        if (errorBlob && errorBlob->GetBufferSize())
        {
            ATOM_ENGINE_ERROR("ShaderCompiler: {}", (char*)errorBlob->GetBufferPointer());
            return false;
        }

        compiledData.resize(shaderBlob->GetBufferSize());
        memcpy(compiledData.data(), shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize());

        return true;
    }
}
