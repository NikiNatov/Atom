#include "atompch.h"
#include "Material.h"

#include "Atom/Renderer/Renderer.h"
#include "Atom/Asset/AssetManager.h"

namespace Atom
{
    // -------------------------------------------------------- Material -----------------------------------------------------------
    // -----------------------------------------------------------------------------------------------------------------------------
    Material::Material(const Ref<GraphicsShader>& shader, MaterialFlags flags)
        : Asset(AssetType::Material), m_Shader(shader), m_Flags(flags)
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
    Material::Material(Material&& rhs) noexcept
        : Asset(AssetType::Material), m_Shader(std::move(rhs.m_Shader)), m_UniformBuffersData(std::move(rhs.m_UniformBuffersData)), m_Textures(std::move(rhs.m_Textures)), m_Flags(rhs.m_Flags)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Material& Material::operator=(Material&& rhs) noexcept
    {
        if (this != &rhs)
        {
            m_Shader = std::move(rhs.m_Shader);
            m_UniformBuffersData = std::move(rhs.m_UniformBuffersData);
            m_Textures = std::move(rhs.m_Textures);
            m_Flags = rhs.m_Flags;
        }

        return *this;
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
    bool Material::HasResource(const char* name, ShaderResourceType type)
    {
        if (const Resource* resource = FindResourceDeclaration(name))
            return resource->Type == type;

        return false;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool Material::HasUniform(const char* name, ShaderDataType type)
    {
        if (const Uniform* uniform = FindUniformDeclaration(name))
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

    // -------------------------------------------------- MaterialTable ------------------------------------------------------------
    // -----------------------------------------------------------------------------------------------------------------------------
    void MaterialTable::SetMaterial(u32 submeshIdx, Ref<Material> material)
    {
        m_Materials[submeshIdx] = material;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool MaterialTable::HasMaterial(u32 submeshIdx) const
    {
        return m_Materials.find(submeshIdx) != m_Materials.end();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<Material> MaterialTable::GetMaterial(u32 submeshIdx) const
    {
        if (!HasMaterial(submeshIdx))
            return nullptr;

        return m_Materials.at(submeshIdx);
    }
}
