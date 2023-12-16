#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Core/DirectX12/DirectX12.h"
#include "Atom/Renderer/DescriptorHeap.h"
#include "Atom/Renderer/ShaderReflection.h"

namespace Atom
{
    enum class ShaderType
    {
        None = 0,
        Graphics,
        Compute,
    };

    class Shader
    {
    public:
        virtual ~Shader() = default;

        inline const String& GetName() const { return m_Name; }
        inline u64 GetHash() const { return m_Hash; }
        inline const ShaderReflection& GetShaderReflection() const { return m_ShaderReflection; }
        inline ShaderType GetShaderType() const { return m_ShaderType; }
    protected:
        Shader(const String& name, const Vector<byte>& vsData, const Vector<byte>& psData);
        Shader(const String& name, const Vector<byte>& csData);
    protected:
        String           m_Name;
        u64              m_Hash;
        ShaderType       m_ShaderType;
        ShaderReflection m_ShaderReflection;
    };

    class GraphicsShader : public Shader
    {
    public:
        GraphicsShader(const String& name, const Vector<byte>& vsData, const Vector<byte>& psData);
        ~GraphicsShader();

        inline const Vector<byte>& GetVSData() const { return m_VSData; }
        inline const Vector<byte>& GetPSData() const { return m_PSData; }
        inline const D3D12_SHADER_BYTECODE& GetD3DVSByteCode() const { return m_D3DVSByteCode; }
        inline const D3D12_SHADER_BYTECODE& GetD3DPSByteCode() const { return m_D3DPSByteCode; }
    private:
        Vector<byte>          m_VSData;
        Vector<byte>          m_PSData;
        D3D12_SHADER_BYTECODE m_D3DVSByteCode;
        D3D12_SHADER_BYTECODE m_D3DPSByteCode;
    };

    class ComputeShader : public Shader
    {
    public:
        ComputeShader(const String& name, const Vector<byte>& csData);
        ~ComputeShader();

        inline const Vector<byte>& GetCSData() const { return m_CSData; }
        inline const D3D12_SHADER_BYTECODE& GetD3DCSByteCode() const { return m_D3DCSByteCode; }
    private:
        Vector<byte>          m_CSData;
        D3D12_SHADER_BYTECODE m_D3DCSByteCode;
    };
}