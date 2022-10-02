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

        void Add(const String& name, const Ref<Shader>& shader);
        void Add(const Ref<Shader>& shader);
        Ref<Shader> Load(const String& name, const String& filepath);
        Ref<Shader> Load(const String& filepath);
        Ref<Shader> Get(const String& name) const;
        bool Exists(const String& name) const;
    private:
        HashMap<String, Ref<Shader>> m_ShaderMap;
    };
}