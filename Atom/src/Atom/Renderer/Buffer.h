#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Core/DirectX12/DirectX12.h"

namespace Atom
{
    // ----------------------------------------------------------------------- Vertex Buffer ----------------------------------------------------------------------- //
    struct VertexBufferDescription
    {
        u32 VertexCount;
        u32 VertexStride;
        const void* Data = nullptr;
    };

    class VertexBuffer
    {
    public:
        VertexBuffer(const VertexBufferDescription& description, const char* debugName = "Unnamed Vertex Buffer");
        ~VertexBuffer();

        inline u32 GetVertexCount() const { return m_Description.VertexCount; }
        inline u32 GetVertexStride() const { return m_Description.VertexStride; }
        inline u32 GetSize() const { return m_Description.VertexCount * m_Description.VertexStride; }
        inline ComPtr<ID3D12Resource> GetD3DResource() const { return m_D3DResource; }
        inline const D3D12_VERTEX_BUFFER_VIEW& GetBufferView() const { return m_View; }
    private:
        VertexBufferDescription  m_Description;
        ComPtr<ID3D12Resource>   m_D3DResource;
        D3D12_VERTEX_BUFFER_VIEW m_View;
    };

    // ----------------------------------------------------------------------- Index Buffer ----------------------------------------------------------------------- //
    enum class IndexBufferFormat
    {
        U16,
        U32
    };

    struct IndexBufferDescription
    {
        u32               IndexCount;
        IndexBufferFormat Format;
        const void*       Data;
    };

    class IndexBuffer
    {
    public:
        IndexBuffer(const IndexBufferDescription& description, const char* debugName = "Unnamed Index Buffer");
        ~IndexBuffer();

        u32 GetIndexCount() const { return m_Description.IndexCount; }
        IndexBufferFormat GetFormat() const { return m_Description.Format; }
        u32 GetSize() const { return m_Description.IndexCount * (m_Description.Format == IndexBufferFormat::U16 ? sizeof(u16) : sizeof(u32)); }
        inline ComPtr<ID3D12Resource> GetD3DResource() const { return m_D3DResource; }
        inline const D3D12_INDEX_BUFFER_VIEW& GetBufferView() const { return m_View; }
    private:
        IndexBufferDescription  m_Description;
        ComPtr<ID3D12Resource>  m_D3DResource;
        D3D12_INDEX_BUFFER_VIEW m_View;
    };
}