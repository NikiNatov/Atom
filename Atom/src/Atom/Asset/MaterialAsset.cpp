#include "atompch.h"
#include "MaterialAsset.h"

#include "Atom/Renderer/Renderer.h"
#include "Atom/Renderer/EngineResources.h"

namespace Atom
{
    // -------------------------------------------------- MaterialAsset ------------------------------------------------------------
    // -----------------------------------------------------------------------------------------------------------------------------
    Material::Material(const Ref<GraphicsShader>& shader, MaterialFlags flags)
        : Asset(AssetType::Material), m_Shader(shader), m_Flags(flags)
    {
        const ShaderLayout& shaderLayout = m_Shader->GetShaderLayout();
        const ShaderLayout::ShaderConstants& constants = shaderLayout.GetConstants(ShaderBindPoint::Material);
        const ShaderLayout::ShaderDescriptorTable& resourceTable = shaderLayout.GetResourceDescriptorTable(ShaderBindPoint::Material);
        const ShaderLayout::ShaderDescriptorTable& samplerTable = shaderLayout.GetSamplerDescriptorTable(ShaderBindPoint::Material);

        // Create constants data storage
        m_ConstantsData.resize(constants.TotalSize, 0);

        // Create textures array
        for (const auto& resource : resourceTable.Resources)
        {
            if (resource.Type == ShaderResourceType::Texture2D || resource.Type == ShaderResourceType::TextureCube)
                m_Textures[resource.Register] = nullptr;
        }

        // Create material SIG
        m_MaterialSIG = CreateRef<CustomShaderInputGroup>(ShaderBindPoint::Material, constants.TotalSize, m_Textures.size(), samplerTable.Resources.size());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Material::SetTexture(const char* name, const Ref<TextureAsset>& texture)
    {
        const ShaderLayout::ShaderResource* textureResource = FindTextureDeclaration(name);
        if (!textureResource)
        {
            ATOM_ERROR("Texture with name \"{}\" does not exist", name);
            return;
        }

        if (m_Textures[textureResource->Register]->GetUUID() == texture->GetUUID())
            return;

        m_Textures[textureResource->Register] = texture;
        m_TexturesDirty = true;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const Ref<TextureAsset>& Material::GetTexture(const char* name)
    {
        const ShaderLayout::ShaderResource* resource = FindTextureDeclaration(name);
        if (!resource)
        {
            ATOM_ERROR("Texture with name \"{}\" does not exist", name);
            return nullptr;
        }

        return m_Textures[resource->Register];
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool Material::HasTexture(const char* name)
    {
        return FindTextureDeclaration(name) != nullptr;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Material::UpdateForRendering()
    {
        m_MaterialSIG->SetConstantData(0, m_ConstantsData.data(), m_ConstantsData.size());

        if (m_TexturesDirty)
        {
            for (auto& [slot, texture] : m_Textures)
            {
                m_MaterialSIG->SetROTexture(slot, texture ? texture->GetResource() : EngineResources::BlackTexture);
                m_MaterialSIG->SetSampler(slot, texture ? Renderer::GetSampler(texture->GetFilter(), texture->GetWrap()) : EngineResources::LinearClampSampler);
            }

            m_TexturesDirty = false;
        }

        m_MaterialSIG->Compile();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool Material::HasUniform(const char* name, ShaderDataType type)
    {
        if (const ShaderLayout::Uniform* uniform = FindUniformDeclaration(name))
            return uniform->Type == type;

        return false;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Material::SetFlag(MaterialFlags flag, bool state)
    {
        if (state)
            m_Flags |= flag;
        else
            m_Flags &= ~flag;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const ShaderLayout::Uniform* Material::FindUniformDeclaration(const char* name)
    {
        const auto& constants = m_Shader->GetShaderLayout().GetConstants(ShaderBindPoint::Material);

        for (const auto& uniform : constants.Uniforms)
        {
            if (uniform.Name == name)
            {
                return &uniform;
            }
        }

        return nullptr;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const ShaderLayout::ShaderResource* Material::FindTextureDeclaration(const char* name)
    {
        const ShaderLayout::ShaderDescriptorTable& resourceTable = m_Shader->GetShaderLayout().GetResourceDescriptorTable(ShaderBindPoint::Material);

        for (const ShaderLayout::ShaderResource& resource : resourceTable.Resources)
        {
            if (resource.Name == name && (resource.Type == ShaderResourceType::Texture2D || resource.Type == ShaderResourceType::TextureCube))
            {
                return &resource;
            }
        }

        return nullptr;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const ShaderLayout::ShaderResource* Material::FindSamplerDeclaration(const char* name)
    {
        const ShaderLayout::ShaderDescriptorTable& samplerTable = m_Shader->GetShaderLayout().GetSamplerDescriptorTable(ShaderBindPoint::Material);

        for (const ShaderLayout::ShaderResource& resource : samplerTable.Resources)
        {
            if (resource.Name == name)
            {
                return &resource;
            }
        }

        return nullptr;
    }

    // -------------------------------------------------- MaterialTable ------------------------------------------------------------
    // -----------------------------------------------------------------------------------------------------------------------------
    void MaterialTable::SetMaterial(u32 submeshIdx, const Ref<Material>& material)
    {
        m_Materials[submeshIdx] = material;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool MaterialTable::HasMaterial(u32 submeshIdx) const
    {
        return m_Materials.find(submeshIdx) != m_Materials.end();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const Ref<Material>& MaterialTable::GetMaterial(u32 submeshIdx) const
    {
        if (!HasMaterial(submeshIdx))
            return nullptr;

        return m_Materials.at(submeshIdx);
    }
}
