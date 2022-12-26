#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Renderer/Shader.h"
#include "Atom/Renderer/Texture.h"

namespace Atom
{
    enum class MaterialFlags
    {
        None = 0,
        DepthTested = BIT(0),
        Wireframe = BIT(1),
        TwoSided = BIT(2),
        Transparent = BIT(3)
    };

    IMPL_ENUM_OPERATORS(MaterialFlags)

    class Material
    {
        using Uniform = ShaderResourceLayout::ShaderUniform;
        using Resource = ShaderResourceLayout::ShaderResource;
        friend class MaterialAsset;
    public:
        Material(const Ref<GraphicsShader>& shader, MaterialFlags flags);
        ~Material() = default;

        void SetTexture(const char* uniformName, const Ref<Texture>& texture);
        Ref<Texture> GetTexture(const char* uniformName);
        bool HasTexture(const char* name);

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

        bool HasUniform(const char* name);

        void SetFlag(MaterialFlags flag, bool state);
        inline void SetFlags(MaterialFlags flags) { m_Flags = flags; }
        inline bool GetFlag(MaterialFlags flag) const { return (m_Flags & flag) != MaterialFlags::None; }
        inline Ref<GraphicsShader> GetShader() const { return m_Shader; }
        inline MaterialFlags GetFlags() const { return m_Flags; }
        inline const HashMap<u32, Vector<byte>>& GetUniformBuffersData() const { return m_UniformBuffersData; }
        inline const HashMap<u32, Ref<Texture>>& GetTextures() const { return m_Textures; }
    private:
        const Uniform* FindUniformDeclaration(const char* name);
        const Resource* FindResourceDeclaration(const char* name);
    private:
        Ref<GraphicsShader>        m_Shader;
        MaterialFlags              m_Flags;
        HashMap<u32, Vector<byte>> m_UniformBuffersData;
        HashMap<u32, Ref<Texture>> m_Textures;
    };

    class MaterialAsset : public Asset
    {
        friend class AssetSerializer;
        friend class ContentTools;
    public:
        MaterialAsset(const Ref<GraphicsShader>& shader);
        ~MaterialAsset() = default;

        template<typename T>
        void SetUniform(const char* uniformName, const T& value) { m_Material->SetUniform(uniformName, value); }
        void SetTexture(const char* uniformName, UUID textureHandle);
        void SetFlag(MaterialFlags flag, bool state) { m_Material->SetFlag(flag, state); }
        void SetFlags(MaterialFlags flags) { m_Material->SetFlags(flags); }

        template<typename T>
        T& GetUniform(const char* uniformName) const { return m_Material->GetUniform<T>(uniformName); }
        UUID GetTexture(const char* uniformName) const;
        bool GetFlag(MaterialFlags flag) const { return m_Material->GetFlag(flag); }
        MaterialFlags GetFlags() const { return m_Material->GetFlags(); }
        Ref<Material> GetMaterial() { return m_Material; }
        Vector<const ShaderResourceLayout::ShaderUniform*> GetUniformDeclarations() const;
        Vector<const ShaderResourceLayout::ShaderResource*> GetTextureDeclarations() const;
    private:
        // Used in the AssetSerializer
        void SetUniformBufferData(u32 bufferRegister, const Vector<byte>& data) { m_Material->m_UniformBuffersData[bufferRegister] = data; }
        const Vector<byte>& GetUniformBufferData(u32 bufferRegister) { return m_Material->m_UniformBuffersData.at(bufferRegister); }
        void SetTexture(u32 textureRegister, const Ref<Texture>& texture) { m_Material->m_Textures[textureRegister] = texture; }
        Ref<Texture> GetTexture(u32 textureRegister) { return m_Material->m_Textures[textureRegister]; }
    private:
        Ref<Material> m_Material = nullptr;
    };

    class MaterialTable
    {
    public:
        MaterialTable() = default;

        void SetMaterial(u32 submeshIdx, UUID materialHandle);
        bool HasMaterial(u32 submeshIdx) const;
        Ref<MaterialAsset> GetMaterial(u32 submeshIdx) const;
        u32 GetMaterialCount() const { return m_Materials.size(); }
    private:
        HashMap<u32, UUID> m_Materials;
    };
}