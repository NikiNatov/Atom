#pragma once

#include "Atom/Core/Core.h"

namespace Atom
{
    enum class ShaderDataType
    {
        None = 0,
        Unorm4,
        Int, Int2, Int3, Int4,
        Uint, Uint2, Uint3, Uint4,
        Float, Float2, Float3, Float4,
        Bool,
        Mat2, Mat3, Mat4
    };

    enum class ShaderResourceType
    {
        None = 0,
        ConstantBuffer,
        Texture2D, Texture2DArray, TextureCube, RWTexture2D, RWTexture2DArray,
        StructuredBuffer, RWStructuredBuffer,
        Sampler
    };

    struct ShaderConstant
    {
        String Name;
        ShaderDataType Type;
        u32 Register;
        u32 Offset;
        u32 Size;

        ShaderConstant(const String& name, ShaderDataType type, u32 bufferRegister, u32 offset, u32 size)
            : Name(name), Type(type), Register(bufferRegister), Offset(offset), Size(size) {}
    };

    struct ShaderResource
    {
        String Name;
        ShaderResourceType Type;
        u32 Register;

        ShaderResource(const String& name, ShaderResourceType type, u32 shaderRegister)
            : Name(name), Type(type), Register(shaderRegister) {}
    };

    class ShaderReflection
    {
    public:
        ShaderReflection(const Vector<byte>& vsData, const Vector<byte>& psData);
        ShaderReflection(const Vector<byte>& csData);

        inline bool HasConstants(u32 shaderSpace) const { return m_Constants.find(shaderSpace) != m_Constants.end(); }
        inline bool HasResources(u32 shaderSpace) const { return m_Resources.find(shaderSpace) != m_Resources.end(); }
        inline bool HasSamplers(u32 shaderSpace) const { return m_Samplers.find(shaderSpace) != m_Samplers.end(); }

        inline const Vector<ShaderConstant>& GetConstants(u32 shaderSpace) const { return m_Constants.at(shaderSpace); }
        inline const Vector<ShaderResource>& GetResources(u32 shaderSpace) const { return m_Resources.at(shaderSpace); }
        inline const Vector<ShaderResource>& GetSamplers(u32 shaderSpace) const { return m_Samplers.at(shaderSpace); }
    private:
        void Reflect(const Vector<byte>& shaderDataBlob);
    private:
        HashMap<u32, Vector<ShaderConstant>> m_Constants;
        HashMap<u32, Vector<ShaderResource>> m_Resources;
        HashMap<u32, Vector<ShaderResource>> m_Samplers;
    };
}