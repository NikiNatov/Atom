#include "atompch.h"
#include "Material.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    Material::Material(const Shader* shader, MaterialFlags flags, const char* name)
        : m_Name(name), m_Shader(shader), m_Flags(flags)
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
        u32 maxRegister = 0;
        for (const auto& resource : resources)
        {
            maxRegister = std::max(maxRegister, resource.Register);
        }

        m_Textures.resize(maxRegister + 1, nullptr);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Material::SetTexture(const char* uniformName, const Ref<Texture>& texture)
    {
        const Resource* resource = FindResourceDeclaration(uniformName);
        ATOM_ENGINE_ASSERT(resource, fmt::format("Resource with name \"{}\" does not exist!", uniformName));

        m_Textures[resource->Register] = texture;
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
}
