#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Asset/TextureAsset.h"
#include "Atom/Renderer/Shader.h"
#include "Atom/Renderer/ShaderInputGroup.h"

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

    class Material : public Asset
    {
        friend class AssetSerializer;
    public:
        Material(const Ref<GraphicsShader>& shader, MaterialFlags flags);

        template<typename T>
        void SetUniform(const char* name, const T& value)
        {
            const ShaderLayout::Uniform* uniform = FindUniformDeclaration(name);
            if (!uniform)
            {
                ATOM_ERROR("Uniform with name \"{}\" does not exist", name);
                return;
            }

            if (uniform->Size != sizeof(T))
            {
                ATOM_ERROR("Size of uniform with name \"{}\" does not match the size of the value type!", name);
                return;
            }

            if (value == *(T*)(&m_ConstantsData[uniform->Offset]))
                return;

            memcpy(m_ConstantsData.data() + uniform->Offset, &value, uniform->Size);
        }

        template<typename T>
        const T& GetUniform(const char* name)
        {
            const ShaderLayout::Uniform* uniform = FindUniformDeclaration(name);
            if (!uniform)
            {
                ATOM_ERROR("Uniform with name \"{}\" does not exist", name);
                return T();
            }

            if (uniform->Size != sizeof(T))
            {
                ATOM_ERROR("Size of uniform with name \"{}\" does not match the size of the value type!", name);
                return T();
            }

            return *(T*)(&m_ConstantsData[uniform->Offset]);
        }

        bool HasUniform(const char* name, ShaderDataType type);

        void SetTexture(const char* name, const Ref<TextureAsset>& texture);
        const Ref<TextureAsset>& GetTexture(const char* name);
        bool HasTexture(const char* name);

        void UpdateForRendering();

        void SetFlag(MaterialFlags flag, bool state);
        inline void SetFlags(MaterialFlags flags) { m_Flags = flags; }
        inline bool GetFlag(MaterialFlags flag) const { return (m_Flags & flag) != MaterialFlags::None; }
        inline MaterialFlags GetFlags() const { return m_Flags; }
        inline const Ref<GraphicsShader>& GetShader() const { return m_Shader; }
        inline const Vector<byte>& GetConstantsData() const { return m_ConstantsData; }
        inline const Map<u32, Ref<TextureAsset>>& GetTextures() const { return m_Textures; }
        inline const Ref<CustomShaderInputGroup>& GetSIG() const { return m_MaterialSIG; }
    private:
        const ShaderLayout::Uniform* FindUniformDeclaration(const char* name);
        const ShaderLayout::ShaderResource* FindTextureDeclaration(const char* name);
        const ShaderLayout::ShaderResource* FindSamplerDeclaration(const char* name);
    private:
        Ref<GraphicsShader>         m_Shader;
        MaterialFlags               m_Flags;
        Vector<byte>                m_ConstantsData;
        Map<u32, Ref<TextureAsset>> m_Textures;
        Ref<CustomShaderInputGroup> m_MaterialSIG;
        bool                        m_TexturesDirty = false;
    };

    class MaterialTable
    {
    public:
        MaterialTable() = default;

        void SetMaterial(u32 submeshIdx, const Ref<Material>& material);
        bool HasMaterial(u32 submeshIdx) const;
        const Ref<Material>& GetMaterial(u32 submeshIdx) const;
        u32 GetMaterialCount() const { return m_Materials.size(); }
    private:
        HashMap<u32, Ref<Material>> m_Materials;
    };
}