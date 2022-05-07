#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Core/DirectX12/DirectX12.h"

#include "ResourceStateTracker.h"

namespace Atom
{
    class Texture;
    class GraphicsPipeline;
    class Framebuffer;
    class Buffer;
    class VertexBuffer;
    class IndexBuffer;
    class ConstantBuffer;

    class CommandBuffer
    {
    public:
        CommandBuffer(const char* debugName = "Unnamed Command Buffer");
        ~CommandBuffer();

        void Begin();
        void BeginRenderPass(const Framebuffer* framebuffer, bool clear = false);
        void EndRenderPass(const Framebuffer* framebuffer);
        void SetGraphicsPipeline(const GraphicsPipeline* pipeline);
        void SetVertexBuffer(const VertexBuffer* vertexBuffer);
        void SetIndexBuffer(const IndexBuffer* indexBuffer);
        void UploadBufferData(const void* data, u32 size, const Buffer* buffer);
        void SetConstantBuffer(u32 slot, const ConstantBuffer* constantBuffer);
        void DrawIndexed(u32 indexCount);
        void End();

        inline ComPtr<ID3D12GraphicsCommandList6> GetCommandList() const { return m_CommandList; }
        inline ComPtr<ID3D12GraphicsCommandList6> GetPendingCommandList() const { return m_PendingCommandList; }
    private:
        Vector<ComPtr<ID3D12CommandAllocator>> m_Allocators;
        ComPtr<ID3D12GraphicsCommandList6>     m_CommandList;
        Vector<ComPtr<ID3D12CommandAllocator>> m_PendingAllocators;
        ComPtr<ID3D12GraphicsCommandList6>     m_PendingCommandList;
        ResourceStateTracker                   m_ResourceStateTracker;
        Vector<ComPtr<ID3D12Resource>>         m_UploadBuffers;
        bool                                   m_IsRecording = false;
    };
}