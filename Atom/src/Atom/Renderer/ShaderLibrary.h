#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Renderer/ShaderCompiler.h"

namespace Atom
{
    class ShaderLibrary
    {
    public:
        ShaderLibrary() = default;
        ~ShaderLibrary() = default;

        Ref<GraphicsShader> LoadGraphicsShader(const std::filesystem::path& filepath)
        {
            Ref<GraphicsShader> shader = ShaderCompiler::CompileGraphicsShader(filepath);

            if(shader)
                Add<GraphicsShader>(shader);

            return shader;
        }

        Ref<ComputeShader> LoadComputeShader(const std::filesystem::path& filepath)
        {
            Ref<ComputeShader> shader = ShaderCompiler::CompileComputeShader(filepath);

            if (shader)
                Add<ComputeShader>(shader);

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
        template<typename ShaderType>
        void Add(const Ref<ShaderType>& shader)
        {
            bool exists = Exists(shader->GetName());
            ATOM_ENGINE_ASSERT(!exists, "Shader with that name already exists!");

            if (!exists)
                m_ShaderMap[shader->GetName()] = shader;
        }
    private:
        HashMap<String, Ref<Shader>> m_ShaderMap;
    };
}