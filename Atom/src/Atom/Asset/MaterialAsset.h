#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Asset/TextureAsset.h"
#include "Atom/Renderer/Material.h"

namespace Atom
{
    class MaterialAsset : public Asset
    {
        friend class AssetSerializer;
    public:
        MaterialAsset(const Ref<GraphicsShader>& shader, MaterialFlags flags);
        MaterialAsset(const Ref<Material>& materialResource);

        void SetTexture(const char* name, Ref<TextureAsset> texture);
        Ref<TextureAsset> GetTexture(const char* name);
        bool HasTexture(const char* name);

        template<typename T>
        void SetUniform(const char* name, const T& value)
        {
            const Material::Uniform* uniform = m_MaterialResource->FindUniformDeclaration(name);
            if (!uniform)
            {
                ATOM_ERROR("Uniform with name \"{}\" does not exist", name);
                return;
            }

            m_MaterialResource->SetUniform<T>(name, value); 
        }

        template<typename T>
        T GetUniform(const char* name)
        {
            const Material::Uniform* uniform = m_MaterialResource->FindUniformDeclaration(name);
            if (!uniform)
            {
                ATOM_ERROR("Uniform with name \"{}\" does not exist", name);
                return T();
            }

            return m_MaterialResource->GetUniform<T>(name);
        }

        bool HasUniform(const char* name, ShaderDataType type);

        void SetFlag(MaterialFlags flag, bool state);
        void SetFlags(MaterialFlags flags);
        bool GetFlag(MaterialFlags flag) const;
        MaterialFlags GetFlags() const;
        bool IsDirty() const;
        Ref<GraphicsShader> GetShader() const;
        const Vector<byte>& GetConstantsData() const;
        const Map<u32, Ref<TextureAsset>>& GetTextureHandles() const;
        Ref<Material> GetResource() const;
    private:
        // Used only during deserialization
        void SetTexture(u32 slot, Ref<TextureAsset> texture);
    private:
        Ref<Material>               m_MaterialResource;
        Map<u32, Ref<TextureAsset>> m_TextureHandles;
    };

    class MaterialTable
    {
    public:
        MaterialTable() = default;

        void SetMaterial(u32 submeshIdx, Ref<MaterialAsset> material);
        bool HasMaterial(u32 submeshIdx) const;
        Ref<MaterialAsset> GetMaterial(u32 submeshIdx) const;
        u32 GetMaterialCount() const { return m_Materials.size(); }
    private:
        HashMap<u32, Ref<MaterialAsset>> m_Materials;
    };
}