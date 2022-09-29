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
    class DescriptorHeap;
    enum class CommandQueueType;

    class CommandBuffer
    {
    public:
        CommandBuffer(CommandQueueType type, const char* debugName = "Unnamed Command Buffer");
        ~CommandBuffer();

        void Begin();
        void BeginRenderPass(const Framebuffer* framebuffer, bool clear = false);
        void EndRenderPass(const Framebuffer* framebuffer);
        void TransitionResource(const Texture* texture, D3D12_RESOURCE_STATES state);
        void SetGraphicsPipeline(const GraphicsPipeline* pipeline);
        void SetVertexBuffer(const VertexBuffer* vertexBuffer);
        void SetIndexBuffer(const IndexBuffer* indexBuffer);
        void SetGraphicsConstantBuffer(u32 rootParamIndex, const ConstantBuffer* constantBuffer);
        void SetGraphicsRootConstants(u32 rootParamIndex, const void* data, u32 numConstants);
        void SetGraphicsDescriptorTable(u32 rootParamIndex, D3D12_GPU_DESCRIPTOR_HANDLE tableStart);
        void SetDescriptorHeaps(const DescriptorHeap* resourceHeap, const DescriptorHeap* samplerHeap);
        void UploadBufferData(const void* data, const Buffer* buffer);
        void UploadTextureData(const void* data, const Texture* texture);
        void DrawIndexed(u32 indexCount, u32 instanceCount = 1, u32 startIndex = 0, u32 startVertex = 0, u32 startInstance = 0);
        void End();

        inline ComPtr<ID3D12GraphicsCommandList6> GetCommandList() const { return m_CommandList; }
        inline ComPtr<ID3D12GraphicsCommandList6> GetPendingCommandList() const { return m_PendingCommandList; }
    private:
        ComPtr<ID3D12CommandAllocator>     m_Allocator;
        ComPtr<ID3D12GraphicsCommandList6> m_CommandList;
        ComPtr<ID3D12CommandAllocator>     m_PendingAllocator;
        ComPtr<ID3D12GraphicsCommandList6> m_PendingCommandList;
        ResourceStateTracker               m_ResourceStateTracker;
        Vector<ComPtr<ID3D12Resource>>     m_UploadBuffers;
        bool                               m_IsRecording = false;
    };
}