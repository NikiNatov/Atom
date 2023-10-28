#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Renderer/Shader.h"

namespace Atom
{
    struct GraphicsShaderFileHeader
    {
        u64 VertexShaderHash;
        u64 VertexShaderDataSize;
        u64 PixelShaderHash;
        u64 PixelShaderDataSize;
    };

    struct ComputeShaderFileHeader
    {
        u64 ShaderHash;
        u64 ShaderDataSize;
    };

    class ShaderCompiler
    {
    public:
        static Ref<GraphicsShader> CompileGraphicsShader(const std::filesystem::path& filepath);
        static Ref<ComputeShader> CompileComputeShader(const std::filesystem::path& filepath);

        inline static void SetOutputDirectory(const std::filesystem::path& directory) { ms_OutputDirectory = directory; };
        inline static void SetOptimizeShaders(bool value) { ms_OptimizeShaders = value; }

        inline static const std::filesystem::path& GetOutputDirectory() { return ms_OutputDirectory; }
        inline static bool GetOptimizeShaders() { return ms_OptimizeShaders; }
    private:
        static Map<u32, std::ostringstream> PreProcessHlslFile(const std::filesystem::path& filepath);
        static bool Compile(const String& shaderSource, const std::filesystem::path& shaderFilepath, const char* entryPoint, const char* target, Vector<byte>& compiledData);
    private:
        inline static std::filesystem::path ms_OutputDirectory;
        inline static bool                  ms_OptimizeShaders = false;
    };
}