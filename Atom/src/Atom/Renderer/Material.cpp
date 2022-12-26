#include "atompch.h"
#include "Material.h"

#include "Atom/Renderer/Renderer.h"
#include "Atom/Asset/AssetManager.h"

namespace Atom
{
    // -------------------------------------------------------- Material -----------------------------------------------------------
    // -----------------------------------------------------------------------------------------------------------------------------
    Material::Material(const Ref<GraphicsShader>& shader, MaterialFlags flags)
        : m_Shader(shader), m_Flags(flags)
    {
        const auto& resourceLayout = m_Shader->GetResourceLayout();
        const auto& rootConstantBuffers = resourceLayout.GetRootConstants();
        const auto& resources = resourceLayout.GetResources();

        // Create uniform data storage for each root constant buffer
        m_UniformBuffersData.reserve(rootConstantBuffers.size());
        for (u32 i = 0; i < rootConstantBuffers.size(); i++)
        {
            m_UniformBuffersData.insert({ rootConstantBuffers[i].Register, Vector<byte>(rootConstantBuffers[i].Size, 0) });
        }

        // Create textures array
        for (const auto& resource : resources)
        {
            if(resource.Type == ShaderResourceType::Texture2D || resource.Type == ShaderResourceType::TextureCube)
                m_Textures[resource.Register] = nullptr;
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Material::SetTexture(const char* uniformName, const Ref<Texture>& texture)
    {
        const Resource* resource = FindResourceDeclaration(uniformName);
        ATOM_ENGINE_ASSERT(resource, fmt::format("Resource with name \"{}\" does not exist!", uniformName));
        ATOM_ENGINE_ASSERT(resource->Type == ShaderResourceType::Texture2D || resource->Type == ShaderResourceType::TextureCube, fmt::format("Resource with name \"{}\" is not a texture!", uniformName));
        m_Textures[resource->Register] = texture;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<Texture> Material::GetTexture(const char* uniformName)
    {
        const Resource* resource = FindResourceDeclaration(uniformName);
        ATOM_ENGINE_ASSERT(resource, fmt::format("Resource with name \"{}\" does not exist!", uniformName));
        ATOM_ENGINE_ASSERT(resource->Type == ShaderResourceType::Texture2D || resource->Type == ShaderResourceType::TextureCube, fmt::format("Resource with name \"{}\" is not a texture!", uniformName));
        return m_Textures[resource->Register];
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool Material::HasTexture(const char* name)
    {
        if (const Resource* resource = FindResourceDeclaration(name))
            return resource->Type == ShaderResourceType::Texture2D || resource->Type == ShaderResourceType::TextureCube;

        return false;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool Material::HasUniform(const char* name)
    {
        return FindUniformDeclaration(name) != nullptr;
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
    const Material::Uniform* Material::FindUniformDeclaration(const char* name)
    {
        const auto& resourceLayout = m_Shader->GetResourceLayout();

        for (const auto& cb : resourceLayout.GetRootConstants())
        {
            for (const auto& uniform : cb.Uniforms)
            {
                if (uniform.Name == name)
                {
                    return &uniform;
                }
            }
        }

        return nullptr;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const Material::Resource* Material::FindResourceDeclaration(const char* name)
    {
        const auto& resourceLayout = m_Shader->GetResourceLayout();

        for (const auto& resource : resourceLayout.GetResources())
        {
            if (resource.Name == name)
            {
                return &resource;
            }
        }

        return nullptr;
    }

    // --------------------------------------------------- MaterialAsset -----------------------------------------------------------
    // -----------------------------------------------------------------------------------------------------------------------------
    MaterialAsset::MaterialAsset(const Ref<GraphicsShader>& shader)
        : Asset(AssetType::Material)
    {
        m_Material = CreateRef<Material>(shader, MaterialFlags::DepthTested);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void MaterialAsset::SetTexture(const char* uniformName, UUID textureHandle)
    {
        if (textureHandle != 0)
        {
            const Material::Resource* resource = m_Material->FindResourceDeclaration(uniformName);

            if (!resource || !(resource->Type == ShaderResourceType::Texture2D || resource->Type == ShaderResourceType::TextureCube))
            {
                ATOM_ERROR("Resource with name \"{}\" does not exist in material or is not a texture", uniformName);
                return;
            }

            switch (resource->Type)
            {
                case ShaderResourceType::Texture2D:
                {
                    Ref<Texture2D> textureAsset = AssetManager::GetAsset<Texture2D>(textureHandle, true);
                    m_Material->SetTexture(uniformName, textureAsset);
                    break;
                }
                case ShaderResourceType::TextureCube:
                {
                    Ref<TextureCube> textureAsset = AssetManager::GetAsset<TextureCube>(textureHandle, true);
                    m_Material->SetTexture(uniformName, textureAsset);
                    break;
                }
            }
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    UUID MaterialAsset::GetTexture(const char* uniformName) const
    {
        const Material::Resource* resource = m_Material->FindResourceDeclaration(uniformName);

        if (!resource || !(resource->Type == ShaderResourceType::Texture2D || resource->Type == ShaderResourceType::TextureCube))
        {
            ATOM_ERROR("Resource with name \"{}\" does not exist in material or is not a texture", uniformName);
            return 0;
        }

        Ref<Texture> texture = m_Material->GetTexture(uniformName);
        if (texture)
        {
            switch (resource->Type)
            {
                case ShaderResourceType::Texture2D:
                {
                    Ref<Texture2D> texture2D = std::dynamic_pointer_cast<Texture2D>(texture);
                    ATOM_ENGINE_ASSERT(texture2D);
                    return texture2D->GetUUID();
                }
                case ShaderResourceType::TextureCube:
                {
                    Ref<TextureCube> textureCube = std::dynamic_pointer_cast<TextureCube>(texture);
                    ATOM_ENGINE_ASSERT(textureCube);
                    return textureCube->GetUUID();
                }
            }
        }

        return 0;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Vector<const ShaderResourceLayout::ShaderUniform*> MaterialAsset::GetUniformDeclarations() const
    {
        Vector<const ShaderResourceLayout::ShaderUniform*> uniformDeclarations;

        for (const auto& cb : m_Material->m_Shader->GetResourceLayout().GetRootConstants())
        {
            for (const auto& uniform : cb.Uniforms)
            {
                uniformDeclarations.push_back(&uniform);
            }
        }

        return uniformDeclarations;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Vector<const ShaderResourceLayout::ShaderResource*> MaterialAsset::GetTextureDeclarations() const
    {
        Vector<const ShaderResourceLayout::ShaderResource*> textureDeclarations;

        for (const auto& resource : m_Material->m_Shader->GetResourceLayout().GetResources())
        {
            if (resource.Type == ShaderResourceType::Texture2D || resource.Type == ShaderResourceType::TextureCube)
            {
                textureDeclarations.push_back(&resource);
            }
        }

        return textureDeclarations;
    }

    // -------------------------------------------------- MaterialTable ------------------------------------------------------------
    // -----------------------------------------------------------------------------------------------------------------------------
    void MaterialTable::SetMaterial(u32 submeshIdx, UUID materialHandle)
    {
        m_Materials[submeshIdx] = materialHandle;
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

        return AssetManager::GetAsset<MaterialAsset>(m_Materials.at(submeshIdx), true);
    }
}
