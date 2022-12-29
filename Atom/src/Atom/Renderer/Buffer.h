#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Core/DirectX12/DirectX12.h"

namespace Atom
{
    enum class BufferType
    {
        None = 0,
        VertexBuffer,
        IndexBuffer,
        ConstantBuffer,
        StructuredBuffer,
        ReadbackBuffer
    };

    struct BufferDescription
    {
        u32 ElementSize = 0;
        u32 ElementCount = 0;
        bool IsDynamic = false;
    };

    class Buffer
    {
    public:
        Buffer(BufferType type, const BufferDescription& description, const char* debugName = "Unnamed Buffer");
        virtual ~Buffer();

        Buffer(const Buffer& rhs) = delete;
        Buffer& operator=(const Buffer& rhs) = delete;

        Buffer(Buffer&& rhs) noexcept;
        Buffer& operator=(Buffer&& rhs) noexcept;

        void* Map(u32 rangeBegin, u32 rangeEnd);
        void Unmap();
        BufferType GetType() const;
        u32 GetElementSize() const;
        u32 GetElementCount() const;
        u32 GetSize() const;
        bool IsDynamic() const;
        inline ComPtr<ID3D12Resource> GetD3DResource() const { return m_D3DResource; }

    protected:
        BufferDescription      m_Description;
        BufferType             m_Type;
        ComPtr<ID3D12Resource> m_D3DResource;
        D3D12_RANGE            m_MapRange;
    };

    class VertexBuffer : public Buffer
    {
    public:
        VertexBuffer(const BufferDescription& description, const char* debugName = "Unnamed Vertex Buffer");
        ~VertexBuffer();

        VertexBuffer(const VertexBuffer& rhs) = delete;
        VertexBuffer& operator=(const VertexBuffer& rhs) = delete;

        VertexBuffer(VertexBuffer&& rhs) noexcept;
        VertexBuffer& operator=(VertexBuffer&& rhs) noexcept;

        inline const D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView() const { return m_View; }
    private:
        virtual void CreateViews();
    private:
        D3D12_VERTEX_BUFFER_VIEW m_View {};
    };

    enum class IndexBufferFormat
    {
        U16,
        U32
    };

    class IndexBuffer : public Buffer
    {
    public:
        IndexBuffer(const BufferDescription& description, IndexBufferFormat format, const char* debugName = "Unnamed Index Buffer");
        ~IndexBuffer();

        IndexBuffer(const IndexBuffer& rhs) = delete;
        IndexBuffer& operator=(const IndexBuffer& rhs) = delete;

        IndexBuffer(IndexBuffer&& rhs) noexcept;
        IndexBuffer& operator=(IndexBuffer&& rhs) noexcept;

        IndexBufferFormat GetFormat() const;
        inline const D3D12_INDEX_BUFFER_VIEW& GetIndexBufferView() const { return m_View; }
    private:
        virtual void CreateViews();
    private:
        D3D12_INDEX_BUFFER_VIEW m_View {};
        IndexBufferFormat       m_Format;
    };

    class ConstantBuffer : public Buffer
    {
    public:
        ConstantBuffer(const BufferDescription& description, const char* debugName = "Unnamed Constant Buffer");
        ~ConstantBuffer();

        ConstantBuffer(const ConstantBuffer& rhs) = delete;
        ConstantBuffer& operator=(const ConstantBuffer& rhs) = delete;

        ConstantBuffer(ConstantBuffer&& rhs) noexcept;
        ConstantBuffer& operator=(ConstantBuffer&& rhs) noexcept;

        inline D3D12_CPU_DESCRIPTOR_HANDLE GetCBV() const { return m_CBVDescriptor; }
    private:
        virtual void CreateViews();
    private:
        D3D12_CPU_DESCRIPTOR_HANDLE m_CBVDescriptor;
    };

    class StructuredBuffer : public Buffer
    {
    public:
        StructuredBuffer(const BufferDescription& description, const char* debugName = "Unnamed Structured Buffer");
        ~StructuredBuffer();

        StructuredBuffer(const StructuredBuffer& rhs) = delete;
        StructuredBuffer& operator=(const StructuredBuffer& rhs) = delete;

        StructuredBuffer(StructuredBuffer&& rhs) noexcept;
        StructuredBuffer& operator=(StructuredBuffer&& rhs) noexcept;

        inline D3D12_CPU_DESCRIPTOR_HANDLE GetSRV() const { return m_SRVDescriptor; }
    private:
        virtual void CreateViews();
    private:
        D3D12_CPU_DESCRIPTOR_HANDLE m_SRVDescriptor;
    };

    class ReadbackBuffer : public Buffer
    {
    public:
        ReadbackBuffer(const BufferDescription& description, const char* debugName = "Unnamed Readback Buffer");

        ReadbackBuffer(const ReadbackBuffer& rhs) = delete;
        ReadbackBuffer& operator=(const ReadbackBuffer& rhs) = delete;
    };
}