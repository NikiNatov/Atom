#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Renderer/Renderer.h"

namespace Atom
{
    // ----------------------------------------------------------------------- Vertex Buffer ----------------------------------------------------------------------- //
    class DX12VertexBuffer;

    struct VertexBufferDescription
    {
        u32 VertexCount;
        u32 VertexStride;
        const void* Data = nullptr;
    };

    class VertexBuffer
    {
    public:
        virtual ~VertexBuffer() = default;

        virtual u32 GetVertexCount() const = 0;
        virtual u32 GetVertexStride() const = 0;
        virtual u32 GetSize() const = 0;

        IMPL_API_CAST(VertexBuffer)

        static Ref<VertexBuffer> Create(const VertexBufferDescription& description, const char* debugName = "Unnamed Vertex Buffer");
    };

    // ----------------------------------------------------------------------- Index Buffer ----------------------------------------------------------------------- //
    class DX12IndexBuffer;

    enum class IndexBufferFormat
    {
        None = 0,
        U16,
        U32
    };

    struct IndexBufferDescription
    {
        u32 IndexCount;
        IndexBufferFormat Format;
        const void* Data;
    };

    class IndexBuffer
    {
    public:
        virtual ~IndexBuffer() = default;

        virtual u32 GetIndexCount() const = 0;
        virtual IndexBufferFormat GetFormat() const = 0;
        virtual u32 GetSize() const = 0;

        IMPL_API_CAST(IndexBuffer)

        static Ref<IndexBuffer> Create(const IndexBufferDescription& description, const char* debugName = "Unnamed Vertex Buffer");
    };
}