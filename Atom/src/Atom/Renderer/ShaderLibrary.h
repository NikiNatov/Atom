#pragma once

#include "Atom/Core/Core.h"

namespace Atom
{
    class Shader;

    class ShaderLibrary
    {
    public:
        ShaderLibrary() = default;
        ~ShaderLibrary() = default;

        template<typename ShaderType>
        void Add(const String& name, const Ref<ShaderType>& shader)
        {
            bool exists = Exists(name);
            ATOM_ENGINE_ASSERT(!exists, "Shader with that name already exists!");

            if (!exists)
                m_ShaderMap[name] = shader;
        }

        template<typename ShaderType>
        void Add(const Ref<ShaderType>& shader)
        {
            auto& name = shader->GetName();
            Add(name, shader);
        }

        template<typename ShaderType>
        Ref<ShaderType> Load(const String& name, const std::filesystem::path& filepath)
        {
            Ref<ShaderType> shader = CreateRef<ShaderType>(filepath);
            Add(name, shader);
            return shader;
        }

        template<typename ShaderType>
        Ref<ShaderType> Load(const std::filesystem::path& filepath)
        {
            Ref<ShaderType> shader = CreateRef<ShaderType>(filepath);
            Add(shader);
            return shader;
        }

        template<typename ShaderType>
        Ref<ShaderType> Get(const String& name) const
        {
            ATOM_ENGINE_ASSERT(Exists(name), "Shader does not exist!");
            Ref<ShaderType> shader = std::dynamic_pointer_cast<ShaderType>(m_ShaderMap.at(name));
            ATOM_ENGINE_ASSERT(shader, "Incorrect shader type!");
            return shader;
        }

        void Clear();
        bool Exists(const String& name) const;
    private:
        HashMap<String, Ref<Shader>> m_ShaderMap;
    };
}