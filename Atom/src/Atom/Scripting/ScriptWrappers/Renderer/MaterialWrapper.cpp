#include "atompch.h"
#include "MaterialWrapper.h"

#include "Atom/Renderer/Renderer.h"
#include "Atom/Asset/AssetManager.h"

namespace Atom
{
    namespace ScriptWrappers
    {
        // -----------------------------------------------------------------------------------------------------------------------------
        Material::Material()
        {
            Ref<GraphicsShader> shader = Renderer::GetShaderLibrary().Get<GraphicsShader>("MeshPBRShader");
            m_Material = CreateRef<Atom::Material>(shader, MaterialFlags::DepthTested);
            AssetManager::RegisterAsset(m_Material);
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        Material::Material(const Ref<Atom::Material>& material)
            : m_Material(material)
        {
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        void Material::SetInt(const String& uniformName, s32 value)
        {
            if (m_Material)
            {
                if (!HasInt(uniformName))
                {
                    ATOM_ERROR("Integer with name \"{}\" doesn't exist", uniformName);
                    return;
                }

                m_Material->SetUniform(uniformName.c_str(), value);
            }
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        void Material::SetFloat(const String& uniformName, f32 value)
        {
            if (m_Material)
            {
                if (!HasFloat(uniformName))
                {
                    ATOM_ERROR("Float with name \"{}\" doesn't exist", uniformName);
                    return;
                }

                m_Material->SetUniform(uniformName.c_str(), value);
            }
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        void Material::SetVec2(const String& uniformName, const glm::vec2& value)
        {
            if (m_Material)
            {
                if (!HasVec2(uniformName))
                {
                    ATOM_ERROR("Vec2 with name \"{}\" doesn't exist", uniformName);
                    return;
                }

                m_Material->SetUniform(uniformName.c_str(), value);
            }
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        void Material::SetVec3(const String& uniformName, const glm::vec3& value)
        {
            if (m_Material)
            {
                if (!HasVec3(uniformName))
                {
                    ATOM_ERROR("Vec3 with name \"{}\" doesn't exist", uniformName);
                    return;
                }

                m_Material->SetUniform(uniformName.c_str(), value);
            }
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        void Material::SetVec4(const String& uniformName, const glm::vec4& value)
        {
            if (m_Material)
            {
                if (!HasVec4(uniformName))
                {
                    ATOM_ERROR("Vec4 with name \"{}\" doesn't exist", uniformName);
                    return;
                }

                m_Material->SetUniform(uniformName.c_str(), value);
            }
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        void Material::SetTexture2D(const String& uniformName, Texture2D texture)
        {
            if (m_Material)
            {
                if (!HasTexture2D(uniformName))
                {
                    ATOM_ERROR("Texture2D with name \"{}\" doesn't exist", uniformName);
                    return;
                }

                m_Material->SetTexture(uniformName.c_str(), texture.GetTexture());
            }
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        void Material::SetTextureCube(const String& uniformName, TextureCube texture)
        {
            if (m_Material)
            {
                if (!HasTextureCube(uniformName))
                {
                    ATOM_ERROR("TextureCube with name \"{}\" doesn't exist", uniformName);
                    return;
                }

                m_Material->SetTexture(uniformName.c_str(), texture.GetTexture());
            }
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        void Material::SetFlag(MaterialFlags flag, bool state)
        {
            if (m_Material)
            {
                m_Material->SetFlag(flag, state);
            }
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        s32 Material::GetInt(const String& uniformName) const
        {
            if (m_Material)
            {
                if (HasInt(uniformName))
                {
                    return m_Material->GetUniform<s32>(uniformName.c_str());
                }
                else
                {
                    ATOM_ERROR("Int with name \"{}\" doesn't exist", uniformName);
                }
            }

            return 0;
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        f32 Material::GetFloat(const String& uniformName) const
        {
            if (m_Material)
            {
                if (HasFloat(uniformName))
                {
                    return m_Material->GetUniform<f32>(uniformName.c_str());
                }
                else
                {
                    ATOM_ERROR("Float with name \"{}\" doesn't exist", uniformName);
                }
            }

            return 0.0f;
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        glm::vec2 Material::GetVec2(const String& uniformName) const
        {
            if (m_Material)
            {
                if (HasVec2(uniformName))
                {
                    return m_Material->GetUniform<glm::vec2>(uniformName.c_str());
                }
                else
                {
                    ATOM_ERROR("Vec2 with name \"{}\" doesn't exist", uniformName);
                }
            }

            return glm::vec2(0.0f);
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        glm::vec3 Material::GetVec3(const String& uniformName) const
        {
            if (m_Material)
            {
                if (HasVec3(uniformName))
                {
                    return m_Material->GetUniform<glm::vec3>(uniformName.c_str());
                }
                else
                {
                    ATOM_ERROR("Vec3 with name \"{}\" doesn't exist", uniformName);
                }
            }

            return glm::vec3(0.0f);
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        glm::vec4 Material::GetVec4(const String& uniformName) const
        {
            if (m_Material)
            {
                if (HasVec4(uniformName))
                {
                    return m_Material->GetUniform<glm::vec4>(uniformName.c_str());
                }
                else
                {
                    ATOM_ERROR("Vec4 with name \"{}\" doesn't exist", uniformName);
                }
            }

            return glm::vec4(0.0f);
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        Texture2D Material::GetTexture2D(const String& uniformName) const
        {
            if (m_Material)
            {
                if (HasTexture2D(uniformName))
                {
                    Ref<Atom::Texture> texture = m_Material->GetTexture(uniformName.c_str());
                    return Texture2D(std::dynamic_pointer_cast<Atom::Texture2D>(texture));
                }
                else
                {
                    ATOM_ERROR("Texture2D with name \"{}\" doesn't exist", uniformName);
                }
            }

            return Texture2D(nullptr);
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        TextureCube Material::GetTextureCube(const String& uniformName) const
        {
            if (m_Material)
            {
                if (HasTextureCube(uniformName))
                {
                    Ref<Atom::Texture> texture = m_Material->GetTexture(uniformName.c_str());
                    return TextureCube(std::dynamic_pointer_cast<Atom::TextureCube>(texture));
                }
                else
                {
                    ATOM_ERROR("TextureCube with name \"{}\" doesn't exist", uniformName);
                }
            }

            return TextureCube(nullptr);
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        bool Material::HasInt(const String& uniformName) const
        {
            if (m_Material)
            {
                return m_Material->HasUniform(uniformName.c_str(), ShaderDataType::Int);
            }

            return false;
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        bool Material::HasFloat(const String& uniformName) const
        {
            if (m_Material)
            {
                return m_Material->HasUniform(uniformName.c_str(), ShaderDataType::Float);
            }

            return false;
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        bool Material::HasVec2(const String& uniformName) const
        {
            if (m_Material)
            {
                return m_Material->HasUniform(uniformName.c_str(), ShaderDataType::Float2);
            }

            return false;
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        bool Material::HasVec3(const String& uniformName) const
        {
            if (m_Material)
            {
                return m_Material->HasUniform(uniformName.c_str(), ShaderDataType::Float3);
            }

            return false;
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        bool Material::HasVec4(const String& uniformName) const
        {
            if (m_Material)
            {
                return m_Material->HasUniform(uniformName.c_str(), ShaderDataType::Float4);
            }

            return false;
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        bool Material::HasTexture2D(const String& uniformName) const
        {
            if (m_Material)
            {
                return m_Material->HasResource(uniformName.c_str(), ShaderResourceType::Texture2D);
            }

            return false;
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        bool Material::HasTextureCube(const String& uniformName) const
        {
            if (m_Material)
            {
                return m_Material->HasResource(uniformName.c_str(), ShaderResourceType::TextureCube);
            }

            return false;
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        bool Material::HasFlag(MaterialFlags flag) const
        {
            if (m_Material)
            {
                return m_Material->GetFlag(flag);
            }

            return false;
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        Ref<Atom::Material> Material::GetMaterial() const
        {
            return m_Material;
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        Material Material::Find(const std::filesystem::path& assetPath)
        {
            UUID uuid = AssetManager::GetUUIDForAssetPath(AssetManager::GetAssetsFolder() / assetPath);

            if (uuid == 0)
                return Material(nullptr);

            return Material(AssetManager::GetAsset<Atom::Material>(uuid, true));
        }
    }
}