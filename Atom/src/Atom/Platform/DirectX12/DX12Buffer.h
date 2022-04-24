#pragma once

#include "Atom/Renderer/API/Buffer.h"

#if defined(ATOM_PLATFORM_WINDOWS)

#include "DirectX12.h"

namespace Atom
{
    class DX12VertexBuffer : public VertexBuffer
    {
    public:
        DX12VertexBuffer(const VertexBufferDescription& description, const char* debugName = "Unnamed Command Buffer");
        ~DX12VertexBuffer();

        virtual u32 GetVertexCount() const override;
        virtual u32 GetVertexStride() const override;
        virtual u32 GetSize() const override;

        inline ComPtr<ID3D12Resource> GetD3DResource() const { return m_D3DResource; }
        inline const D3D12_VERTEX_BUFFER_VIEW& GetBufferView() const { return m_View; }
    private:
        VertexBufferDescription  m_Description;
        ComPtr<ID3D12Resource>   m_D3DResource;
        D3D12_VERTEX_BUFFER_VIEW m_View;
    };

    class DX12IndexBuffer : public IndexBuffer
    {
    public:
        DX12IndexBuffer(const IndexBufferDescription& description, const char* debugName = "Unnamed Command Buffer");
        ~DX12IndexBuffer();

        virtual u32 GetIndexCount() const override;
        virtual IndexBufferFormat GetFormat() const override;
        virtual u32 GetSize() const override;

        inline ComPtr<ID3D12Resource> GetD3DResource() const { return m_D3DResource; }
        inline const D3D12_INDEX_BUFFER_VIEW& GetBufferView() const { return m_View; }
        inline DXGI_FORMAT GetDXGIFormat() const { return m_DXGIFormat; }
    private:
        IndexBufferDescription  m_Description;
        ComPtr<ID3D12Resource>  m_D3DResource;
        D3D12_INDEX_BUFFER_VIEW m_View;
        DXGI_FORMAT             m_DXGIFormat;
    };
}

#endif // ATOM_PLATFORM_WINDOWS