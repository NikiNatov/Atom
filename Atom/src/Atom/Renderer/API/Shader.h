#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Renderer/Renderer.h"

namespace Atom
{
    class DX12Shader;

    enum class ShaderDataType
    {
        Int, Int2, Int3, Int4,
        Uint, Uint2, Uint3, Uint4,
        Float, Float2, Float3, Float4,
        Bool,
        Mat2, Mat3, Mat4
    };

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