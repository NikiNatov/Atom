#include "atompch.h"
#include "MaterialAsset.h"

namespace Atom
{
    // -------------------------------------------------- MaterialAsset ------------------------------------------------------------
    // -----------------------------------------------------------------------------------------------------------------------------
    MaterialAsset::MaterialAsset(const Ref<GraphicsShader>& shader, MaterialFlags flags)
        : Asset(AssetType::Material)
    {
        m_MaterialResource = CreateRef<Material>(shader, flags);

        for (auto& [slot, _] : m_MaterialResource->GetTextures())
            m_TextureHandles[slot] = 0;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    MaterialAsset::MaterialAsset(const Ref<Material>& materialResource)
        : Asset(AssetType::Material), m_MaterialResource(materialResource)
    {
        for (auto& [slot, _] : m_MaterialResource->GetTextures())
            m_TextureHandles[slot] = 0;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void MaterialAsset::SetTexture(const char* name, Ref<TextureAsset> texture)
    {
        const Material::Resource* resource = m_MaterialResource->FindResourceDeclaration(name);
        if (!resource || resource->Type != ShaderResourceType::Texture2D)
        {
            ATOM_ERROR("Texture with name \"{}\" does not exist", name);
            return;
        }

        m_MaterialResource->SetTexture(name, texture->GetResource());
        m_TextureHandles[resource->Register] = texture;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<TextureAsset> MaterialAsset::GetTexture(const char* name)
    {
        const Material::Resource* resource = m_MaterialResource->FindResourceDeclaration(name);
        if (!resource || resource->Type != ShaderResourceType::Texture2D)
        {
            ATOM_ERROR("Texture with name \"{}\" does not exist", name);
            return nullptr;
        }

        return m_TextureHandles[resource->Register];
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool MaterialAsset::HasTexture(const char* name)
    {
        return m_MaterialResource->HasResource(name, ShaderResourceType::Texture2D);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool MaterialAsset::HasUniform(const char* name, ShaderDataType type)
    {
        return m_MaterialResource->HasUniform(name, type);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void MaterialAsset::SetFlag(MaterialFlags flag, bool state)
    {
        m_MaterialResource->SetFlag(flag, state);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void MaterialAsset::SetFlags(MaterialFlags flags)
    {
        m_MaterialResource->SetFlags(flags);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool MaterialAsset::GetFlag(MaterialFlags flag) const
    {
        return m_MaterialResource->GetFlag(flag);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    MaterialFlags MaterialAsset::GetFlags() const
    {
        return m_MaterialResource->GetFlags();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool MaterialAsset::IsDirty() const
    {
        return m_MaterialResource->IsDirty();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<GraphicsShader> MaterialAsset::GetShader() const
    {
        return m_MaterialResource->GetShader();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const Vector<byte>& MaterialAsset::GetConstantsData() const
    {
        return m_MaterialResource->GetConstantsData();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const Map<u32, Ref<TextureAsset>>& MaterialAsset::GetTextureHandles() const
    {
        return m_TextureHandles;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<Material> MaterialAsset::GetResource() const
    {
        return m_MaterialResource;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void MaterialAsset::SetTexture(u32 slot, Ref<TextureAsset> texture)
    {
        m_MaterialResource->m_Textures[slot] = texture->GetResource();
        m_TextureHandles[slot] = texture;
    }

    // -------------------------------------------------- MaterialTable ------------------------------------------------------------
    // -----------------------------------------------------------------------------------------------------------------------------
    void MaterialTable::SetMaterial(u32 submeshIdx, Ref<MaterialAsset> material)
    {
        m_Materials[submeshIdx] = material;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool MaterialTable::HasMaterial(u32 submeshIdx) const
    {
        return m_Materials.find(submeshIdx) != m_Materials.end();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<MaterialAsset> MaterialTable::GetMaterial(u32 submeshIdx) const
    {
        if (!HasMaterial(submeshIdx))
            return nullptr;

        return m_Materials.at(submeshIdx);
    }
}
