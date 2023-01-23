#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Renderer/Material.h"

#include "Atom/Scripting/ScriptWrappers/Renderer/TextureWrapper.h"

namespace Atom
{
    namespace ScriptWrappers
    {
        class Material
        {
        public:
            Material();
            Material(u64 assetUUID);
            Material(const Ref<Atom::Material>& material);

            void SetInt(const String& uniformName, s32 value);
            void SetFloat(const String& uniformName, f32 value);
            void SetVec2(const String& uniformName, const glm::vec2& value);
            void SetVec3(const String& uniformName, const glm::vec3& value);
            void SetVec4(const String& uniformName, const glm::vec4& value);
            void SetTexture2D(const String& uniformName, Texture2D texture);
            void SetTextureCube(const String& uniformName, TextureCube texture);
            void SetFlag(MaterialFlags flag, bool state);
            s32 GetInt(const String& uniformName) const;
            f32 GetFloat(const String& uniformName) const;
            glm::vec2 GetVec2(const String& uniformName) const;
            glm::vec3 GetVec3(const String& uniformName) const;
            glm::vec4 GetVec4(const String& uniformName) const;
            Texture2D GetTexture2D(const String& uniformName) const;
            TextureCube GetTextureCube(const String& uniformName) const;
            bool HasInt(const String& uniformName) const;
            bool HasFloat(const String& uniformName) const;
            bool HasVec2(const String& uniformName) const;
            bool HasVec3(const String& uniformName) const;
            bool HasVec4(const String& uniformName) const;
            bool HasTexture2D(const String& uniformName) const;
            bool HasTextureCube(const String& uniformName) const;
            bool HasFlag(MaterialFlags flag) const;
            
            UUID GetUUID() const;
            Ref<Atom::Material> GetMaterial() const;

            static Material Find(const std::filesystem::path& assetPath);
        private:
            Ref<Atom::Material> m_Material = nullptr;
        };
    }
}