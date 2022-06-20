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
        ConstantBuffer
    };

    struct BufferDescription
    {
        u32 ElementSize;
        u32 ElementCount;
        const void* Data;
    };

    class Buffer
    {
    public:
        Buffer(BufferType type, const BufferDescription& description, const char* debugName = "Unnamed Buffer");
        virtual ~Buffer();

        void SetData(CommandBuffer* commandBuffer, const void* data, u32 size);
        BufferType GetType() const;
        u32 GetElementSize() const;
        u32 GetElementCount() const;
        u32 GetSize() const;
        inline ComPtr<ID3D12Resource> GetD3DResource() const { return m_D3DResource; }
    protected:
        virtual void CreateViews() = 0;
    protected:
        BufferDescription      m_Description;
        BufferType             m_Type;
        ComPtr<ID3D12Resource> m_D3DResource;
    };

    class VertexBuffer : public Buffer
    {
    public:
        VertexBuffer(const BufferDescription& description, const char* debugName = "Unnamed Vertex Buffer");
        ~VertexBuffer();

        inline const D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView() const { return m_View; }
    private:
        virtual void CreateViews() override;
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

        IndexBufferFormat GetFormat() const;
        inline const D3D12_INDEX_BUFFER_VIEW& GetIndexBufferView() const { return m_View; }
    private:
        virtual void CreateViews() override;
    private:
        D3D12_INDEX_BUFFER_VIEW m_View {};
        IndexBufferFormat       m_Format;
    };

    class ConstantBuffer : public Buffer
    {
    public:
        ConstantBuffer(const BufferDescription& description, const char* debugName = "Unnamed Constant Buffer");
        ~ConstantBuffer();

        inline D3D12_CPU_DESCRIPTOR_HANDLE GetCBV() const { return m_CBVDescriptor; }
    private:
        virtual void CreateViews() override;
    private:
        D3D12_CPU_DESCRIPTOR_HANDLE m_CBVDescriptor;
    };
}