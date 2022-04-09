#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Renderer/Renderer.h"

namespace Atom
{
    class DX12Shader;

    class Shader
    {
    public:
        virtual ~Shader() = default;

        virtual const String& GetName() const = 0;
        virtual const String& GetFilepath() const = 0;
        virtual u64 GetHash() const = 0;

        IMPL_API_CAST(Shader)

        static Ref<Shader> Create(const String& filepath);
    };
}