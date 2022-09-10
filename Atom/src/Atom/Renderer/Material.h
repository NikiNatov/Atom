#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Renderer/Shader.h"
#include "Atom/Renderer/Texture.h"

namespace Atom
{
    enum class MaterialFlags
    {
        None = 0,
        DepthTested,
        Wireframe,
        TwoSided,
        Transparent
    };

    IMPL_ENUM_OPERATORS(MaterialFlags)

    class Material
    {
        using Uniform = ShaderResourceLayout::ShaderUniform;
        using Resource = ShaderResourceLayout::ShaderResource;
    public:
        Material(const Shader* shader, MaterialFlags flags, const char* name);
        ~Material() = default;

        template<typename T>
        void SetUniform(const char* uniformName, const T& value)
        {
            const Uniform* uniform = FindUniformDeclaration(uniformName);

            ATOM_ENGINE_ASSERT(uniform, fmt::format("Uniform with name \"{}\" does not exist!", uniformName));
            ATOM_ENGINE_ASSERT(uniform->Size == sizeof(T), "Uniform size mismatch!");

            Vector<byte>& bufferData = m_UniformBuffersData[uniform->BufferRegister];
            memcpy(bufferData.data() + uniform->Offset, &value, uniform->Size);
        }

        template<typename T>
        inline T& GetUniform(const char* uniformName)
        {
            const Uniform* uniform = FindUniformDeclaration(uniformName);
            ATOM_ENGINE_ASSERT(uniform, fmt::format("Uniform with name \"{}\" does not exist!", uniformName));

            Vector<byte>& bufferData = m_UniformBuffersData[uniform->BufferRegister];
            return *(T*)(&bufferData[uniform->Offset]);
        }
        
        void SetTexture(const char* uniformName, const Ref<Texture>& texture);
        inline void SetFlags(MaterialFlags flags) { m_Flags = flags; }
        inline bool IsDepthTested() const { return (m_Flags & MaterialFlags::DepthTested) != MaterialFlags::None; }
        inline bool IsWireframe() const { return (m_Flags & MaterialFlags::Wireframe) != MaterialFlags::None; }
        inline bool IsTwoSided() const { return (m_Flags & MaterialFlags::TwoSided) != MaterialFlags::None; }
        inline bool IsTransparent() const { return (m_Flags & MaterialFlags::Transparent) != MaterialFlags::None; }
        inline const String& GetName() const { return m_Name; }
        inline const Shader* GetShader() const { return m_Shader; }
        inline MaterialFlags GetFlags() const { return m_Flags; }
        inline const HashMap<u32, Vector<byte>>& GetUniformBuffersData() const { return m_UniformBuffersData; }
        inline const Vector<Ref<Texture>>& GetTextures() const { return m_Textures; }
    private:
        const Uniform* FindUniformDeclaration(const char* name);
        const Resource* FindResourceDeclaration(const char* name);
    private:
        String                     m_Name;
        const Shader*              m_Shader;
        MaterialFlags              m_Flags;
        HashMap<u32, Vector<byte>> m_UniformBuffersData;
        Vector<Ref<Texture>>       m_Textures;
    };
}