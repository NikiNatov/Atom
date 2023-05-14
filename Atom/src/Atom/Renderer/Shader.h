#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Core/DirectX12/DirectX12.h"
#include "Atom/Renderer/DescriptorHeap.h"
#include "Atom/Renderer/ShaderLayout.h"

#include <d3dcompiler.h>

namespace Atom
{
    class Shader
    {
    public:
        virtual ~Shader() = default;

        inline const String& GetName() const { return m_Name; }
        inline const std::filesystem::path& GetFilepath() const { return m_Filepath; }
        inline u64 GetHash() const { return std::hash<String>{}(m_Filepath.string()); }
        inline const ShaderLayout& GetShaderLayout() const { return m_ShaderLayout; }
    protected:
        Shader(const std::filesystem::path& filepath);
    protected:
        String                m_Name;
        std::filesystem::path m_Filepath;
        ShaderLayout          m_ShaderLayout;
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