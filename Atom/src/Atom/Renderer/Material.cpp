#include "atompch.h"
#include "Material.h"

#include "Atom/Renderer/Renderer.h"
#include "Atom/Renderer/Device.h"
#include "Atom/Asset/AssetManager.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    Material::Material(const Ref<GraphicsShader>& shader, MaterialFlags flags)
        : m_Shader(shader), m_Flags(flags)
    {
        const auto& shaderLayout = m_Shader->GetShaderLayout();
        const auto& constants = shaderLayout.GetConstants(ShaderBindPoint::Material);
        const auto& resourceTable = shaderLayout.GetResourceDescriptorTable(ShaderBindPoint::Material);
        const auto& samplerTable = shaderLayout.GetSamplerDescriptorTable(ShaderBindPoint::Material);

        // Create constants data storage
        m_ConstantsData.resize(constants.TotalSize, 0);

        // Create textures array
        for (const auto& resource : resourceTable.Resources)
        {
            if(resource.Type == ShaderResourceType::Texture2D || resource.Type == ShaderResourceType::TextureCube)
                m_Textures[resource.Register] = nullptr;
        }

        // Create sampler array
        for (const auto& resource : samplerTable.Resources)
        {
            if (resource.Type == ShaderResourceType::Sampler)
                m_Samplers[resource.Register] = nullptr;
        }

        UpdateDescriptorTables();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Material::Material(Material&& rhs) noexcept
        : m_Shader(std::move(rhs.m_Shader)), m_ConstantsData(std::move(rhs.m_ConstantsData)), m_Textures(std::move(rhs.m_Textures)), m_Samplers(std::move(rhs.m_Samplers)), m_Flags(rhs.m_Flags)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Material::~Material()
    {
        Device::Get().GetGPUDescriptorHeap(DescriptorHeapType::ShaderResource)->Release(std::move(m_ResourceDescriptorTable), true);
        Device::Get().GetGPUDescriptorHeap(DescriptorHeapType::Sampler)->Release(std::move(m_SamplerDescriptorTable), true);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Material& Material::operator=(Material&& rhs) noexcept
    {
        if (this != &rhs)
        {
            m_Shader = std::move(rhs.m_Shader);
            m_ConstantsData = std::move(rhs.m_ConstantsData);
            m_Textures = std::move(rhs.m_Textures);
            m_Samplers = std::move(rhs.m_Samplers);
            m_Flags = rhs.m_Flags;
        }

        return *this;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Material::UpdateDescriptorTables()
    {
        m_Dirty = false;

        GPUDescriptorHeap* resourceDescriptorHeap = Device::Get().GetGPUDescriptorHeap(DescriptorHeapType::ShaderResource);
        GPUDescriptorHeap* samplerDescriptorHeap = Device::Get().GetGPUDescriptorHeap(DescriptorHeapType::Sampler);

        if (!m_ResourceDescriptorTable.IsValid() || m_ResourceDescriptorTable.GetSize() < m_Textures.size())
        {
            resourceDescriptorHeap->Release(std::move(m_ResourceDescriptorTable), true);
            m_ResourceDescriptorTable = resourceDescriptorHeap->AllocatePersistent(m_Textures.size());
        }

        if (!m_SamplerDescriptorTable.IsValid() || m_SamplerDescriptorTable.GetSize() < m_Textures.size())
        {
            samplerDescriptorHeap->Release(std::move(m_SamplerDescriptorTable), true);
            m_SamplerDescriptorTable = samplerDescriptorHeap->AllocatePersistent(m_Textures.size());
        }

        // Gather and copy the texture and sampler descriptors to the GPU descriptor heap
        Vector<D3D12_CPU_DESCRIPTOR_HANDLE> textureDescriptors;
        textureDescriptors.reserve(m_Textures.size());

        Ref<Texture> errorTexture = Renderer::GetErrorTexture();
        for (const auto& [_, texture] : m_Textures)
        {
            textureDescriptors.push_back(texture ? texture->GetSRV()->GetDescriptor() : errorTexture->GetSRV()->GetDescriptor());
        }

        Vector<D3D12_CPU_DESCRIPTOR_HANDLE> samplerDescriptors;
        samplerDescriptors.reserve(m_Samplers.size());

        Ref<TextureSampler> defaultSampler = Renderer::GetSampler(TextureFilter::Linear, TextureWrap::Clamp);
        for (const auto& [_, sampler] : m_Samplers)
        {
            samplerDescriptors.push_back(sampler ? sampler->GetDescriptor() : defaultSampler->GetDescriptor());
        }

        Device::Get().CopyDescriptors(m_ResourceDescriptorTable, textureDescriptors.size(), textureDescriptors.data(), DescriptorHeapType::ShaderResource);
        Device::Get().CopyDescriptors(m_SamplerDescriptorTable, samplerDescriptors.size(), samplerDescriptors.data(), DescriptorHeapType::Sampler);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Material::SetTexture(const char* uniformName, const Ref<Texture>& texture)
    {
        const Resource* resource = FindResourceDeclaration(uniformName);
        ATOM_ENGINE_ASSERT(resource, fmt::format("Texture with name \"{}\" does not exist!", uniformName));
        ATOM_ENGINE_ASSERT(resource->Type == ShaderResourceType::Texture2D || resource->Type == ShaderResourceType::TextureCube, fmt::format("Texture with name \"{}\" is not a texture!", uniformName));
        m_Textures[resource->Register] = texture;
        m_Dirty = true;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<Texture> Material::GetTexture(const char* uniformName)
    {
        const Resource* resource = FindResourceDeclaration(uniformName);
        ATOM_ENGINE_ASSERT(resource, fmt::format("Texture with name \"{}\" does not exist!", uniformName));
        ATOM_ENGINE_ASSERT(resource->Type == ShaderResourceType::Texture2D || resource->Type == ShaderResourceType::TextureCube, fmt::format("Texture with name \"{}\" is not a texture!", uniformName));
        return m_Textures[resource->Register];
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Material::SetSampler(const char* uniformName, const Ref<TextureSampler>& sampler)
    {
        const Resource* resource = FindResourceDeclaration(uniformName);
        ATOM_ENGINE_ASSERT(resource, fmt::format("Sampler with name \"{}\" does not exist!", uniformName));
        ATOM_ENGINE_ASSERT(resource->Type == ShaderResourceType::Sampler, fmt::format("Sampler with name \"{}\" is not a texture!", uniformName));
        m_Samplers[resource->Register] = sampler;
        m_Dirty = true;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<TextureSampler> Material::GetSampler(const char* uniformName)
    {
        const Resource* resource = FindResourceDeclaration(uniformName);
        ATOM_ENGINE_ASSERT(resource, fmt::format("Sampler with name \"{}\" does not exist!", uniformName));
        ATOM_ENGINE_ASSERT(resource->Type == ShaderResourceType::Sampler, fmt::format("Sampler with name \"{}\" is not a texture!", uniformName));
        return m_Samplers[resource->Register];
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
    const Material::Resource* Material::FindResourceDeclaration(const char* name)
    {
        const auto& resourceTable = m_Shader->GetShaderLayout().GetResourceDescriptorTable(ShaderBindPoint::Material);

        for (const auto& resource : resourceTable.Resources)
        {
            if (resource.Name == name)
            {
                return &resource;
            }
        }

        const auto& samplerTable = m_Shader->GetShaderLayout().GetSamplerDescriptorTable(ShaderBindPoint::Material);

        for (const auto& resource : samplerTable.Resources)
        {
            if (resource.Name == name)
            {
                return &resource;
            }
        }

        return nullptr;
    }
}
