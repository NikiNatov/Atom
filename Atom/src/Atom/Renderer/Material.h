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
        friend class MaterialAsset;
        using Uniform = ShaderLayout::Uniform;
        using Resource = ShaderLayout::ShaderResource;
    public:
        Material(const Ref<GraphicsShader>& shader, MaterialFlags flags);
        ~Material();

        Material(const Material& rhs) = delete;
        Material& operator=(const Material& rhs) = delete;

        Material(Material&& rhs) noexcept;
        Material& operator=(Material&& rhs) noexcept;

        void UpdateDescriptorTables();

        void SetTexture(const char* uniformName, const Ref<Texture>& texture);
        Ref<Texture> GetTexture(const char* uniformName);
        bool HasResource(const char* name, ShaderResourceType type);

        template<typename T>
        void SetUniform(const char* uniformName, const T& value)
        {
            const Uniform* uniform = FindUniformDeclaration(uniformName);

            ATOM_ENGINE_ASSERT(uniform, fmt::format("Uniform with name \"{}\" does not exist!", uniformName));
            ATOM_ENGINE_ASSERT(uniform->Size == sizeof(T), "Uniform size mismatch!");

            memcpy(m_ConstantsData.data() + uniform->Offset, &value, uniform->Size);
        }

        template<typename T>
        inline T& GetUniform(const char* uniformName)
        {
            const Uniform* uniform = FindUniformDeclaration(uniformName);
            ATOM_ENGINE_ASSERT(uniform, fmt::format("Uniform with name \"{}\" does not exist!", uniformName));

            return *(T*)(&m_ConstantsData[uniform->Offset]);
        }

        bool HasUniform(const char* name, ShaderDataType type);

        void SetFlag(MaterialFlags flag, bool state);
        inline void SetFlags(MaterialFlags flags) { m_Flags = flags; }
        inline bool GetFlag(MaterialFlags flag) const { return (m_Flags & flag) != MaterialFlags::None; }
        inline MaterialFlags GetFlags() const { return m_Flags; }
        inline Ref<GraphicsShader> GetShader() const { return m_Shader; }
        inline const Vector<byte>& GetConstantsData() const { return m_ConstantsData; }
        inline const Map<u32, Ref<Texture>>& GetTextures() const { return m_Textures; }
        inline const DescriptorAllocation& GetResourceTable() const { return m_ResourceDescriptorTable; }
        inline const DescriptorAllocation& GetSamplerTable() const { return m_SamplerDescriptorTable; }
        inline bool IsDirty() const { return m_Dirty; }
    private:
        const Uniform* FindUniformDeclaration(const char* name);
        const Resource* FindResourceDeclaration(const char* name);
    private:
        Ref<GraphicsShader>    m_Shader;
        MaterialFlags          m_Flags;
        Vector<byte>           m_ConstantsData;
        Map<u32, Ref<Texture>> m_Textures;
        DescriptorAllocation   m_ResourceDescriptorTable;
        DescriptorAllocation   m_SamplerDescriptorTable;
        bool                   m_Dirty = false;
    };
}