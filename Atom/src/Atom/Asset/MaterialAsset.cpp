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
        const ShaderReflection& shaderReflection = m_Shader->GetShaderReflection();
        const Vector<ShaderConstant>& constants = shaderReflection.GetConstants(MaterialSIG::BindPointType::ShaderSpace);
        const Vector<ShaderResource>& resources = shaderReflection.GetResources(MaterialSIG::BindPointType::ShaderSpace);
        const Vector<ShaderResource>& samplers = shaderReflection.GetSamplers(MaterialSIG::BindPointType::ShaderSpace);

        // Create constants data storage
        u32 cbSize = 0;

        for (const ShaderConstant& constant : constants)
            cbSize += constant.Size;

        m_ConstantsData.resize(cbSize, 0);

        // Create textures array
        for (const ShaderResource& resource : resources)
        {
            if (resource.Type == ShaderResourceType::Texture2D || resource.Type == ShaderResourceType::TextureCube)
                m_Textures[resource.Register] = nullptr;
        }

        // Create SIG
        m_MaterialSIG = CreateRef<MaterialSIG>(cbSize, m_Textures.size(), 0, m_Textures.size(), true);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Material::SetTexture(const char* name, const Ref<TextureAsset>& texture)
    {
        const ShaderResource* textureResource = FindTextureDeclaration(name);
        if (!textureResource)
        {
            ATOM_ERROR("Texture with name \"{}\" does not exist", name);
            return;
        }

        if (m_Textures[textureResource->Register] && m_Textures[textureResource->Register]->GetUUID() == texture->GetUUID())
            return;

        m_Textures[textureResource->Register] = texture;
        m_TexturesDirty = true;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const Ref<TextureAsset>& Material::GetTexture(const char* name)
    {
        const ShaderResource* resource = FindTextureDeclaration(name);
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
        m_MaterialSIG->SetConstant(0, m_ConstantsData.data(), m_ConstantsData.size());

        if (m_TexturesDirty)
        {
            for (auto& [slot, texture] : m_Textures)
            {
                m_MaterialSIG->SetROTexture(slot, texture ? texture->GetResource().get() : EngineResources::BlackTexture.get());
                m_MaterialSIG->SetSampler(slot, texture ? Renderer::GetSampler(texture->GetFilter(), texture->GetWrap()).get() : EngineResources::LinearClampSampler.get());
            }

            m_TexturesDirty = false;
        }

        m_MaterialSIG->Compile();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool Material::HasUniform(const char* name, ShaderDataType type)
    {
        if (const ShaderConstant* uniform = FindUniformDeclaration(name))
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
    const ShaderConstant* Material::FindUniformDeclaration(const char* name)
    {
        const Vector<ShaderConstant>& constants = m_Shader->GetShaderReflection().GetConstants(MaterialSIG::BindPointType::ShaderSpace);

        for (const ShaderConstant& uniform : constants)
        {
            if (uniform.Name == name)
            {
                return &uniform;
            }
        }

        return nullptr;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const ShaderResource* Material::FindTextureDeclaration(const char* name)
    {
        const Vector<ShaderResource>& resources = m_Shader->GetShaderReflection().GetResources(MaterialSIG::BindPointType::ShaderSpace);

        for (const ShaderResource& resource : resources)
        {
            if (resource.Name == name && (resource.Type == ShaderResourceType::Texture2D || resource.Type == ShaderResourceType::TextureCube))
            {
                return &resource;
            }
        }

        return nullptr;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const ShaderResource* Material::FindSamplerDeclaration(const char* name)
    {
        const Vector<ShaderResource>& samplers = m_Shader->GetShaderReflection().GetSamplers(MaterialSIG::BindPointType::ShaderSpace);

        for (const ShaderResource& sampler : samplers)
        {
            if (sampler.Name == name)
            {
                return &sampler;
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
