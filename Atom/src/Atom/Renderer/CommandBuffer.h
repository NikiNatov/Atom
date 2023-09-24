#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Core/DirectX12/DirectX12.h"

#include "Atom/Renderer/ShaderLayout.h"

#include "ResourceStateTracker.h"

namespace Atom
{
    class Texture;
    class GraphicsPipeline;
    class ComputePipeline;
    class Framebuffer;
    class Buffer;
    class VertexBuffer;
    class IndexBuffer;
    class ConstantBuffer;
    class StructuredBuffer;
    class ReadbackBuffer;
    class GPUDescriptorHeap;
    class ComputeShader;
    class ResourceBarrier;
    enum class CommandQueueType;

    class CommandBuffer
    {
    public:
        CommandBuffer(CommandQueueType type, const char* debugName = "Unnamed Command Buffer");
        ~CommandBuffer();

        void Begin();
        void BeginRenderPass(const Framebuffer* framebuffer, bool clear = false);
        void EndRenderPass(const Framebuffer* framebuffer);
        void ApplyBarriers(const Vector<ResourceBarrier*>& barriers);
        void TransitionResource(const Texture* texture, D3D12_RESOURCE_STATES state, u32 subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
        void AddUAVBarrier(const Texture* texture);
        void CommitBarriers();
        void SetVertexBuffer(const VertexBuffer* vertexBuffer);
        void SetIndexBuffer(const IndexBuffer* indexBuffer);
        void SetGraphicsPipeline(const GraphicsPipeline* pipeline);
        void SetComputePipeline(const ComputePipeline* pipeline);
        void SetGraphicsConstants(ShaderBindPoint bindPoint, const ConstantBuffer* constantBuffer);
        void SetComputeConstants(ShaderBindPoint bindPoint, const ConstantBuffer* constantBuffer);
        void SetGraphicsConstants(ShaderBindPoint bindPoint, const void* data, u32 numConstants);
        void SetComputeConstants(ShaderBindPoint bindPoint, const void* data, u32 numConstants);
        void SetGraphicsDescriptorTables(ShaderBindPoint bindPoint, D3D12_GPU_DESCRIPTOR_HANDLE resourceTable, D3D12_GPU_DESCRIPTOR_HANDLE samplerTable = D3D12_GPU_DESCRIPTOR_HANDLE{ 0 });
        void SetComputeDescriptorTables(ShaderBindPoint bindPoint, D3D12_GPU_DESCRIPTOR_HANDLE resourceTable, D3D12_GPU_DESCRIPTOR_HANDLE samplerTable = D3D12_GPU_DESCRIPTOR_HANDLE{ 0 });
        void SetDescriptorHeaps(const GPUDescriptorHeap* resourceHeap, const GPUDescriptorHeap* samplerHeap);
        void UploadBufferData(const void* data, const Buffer* buffer);
        void UploadTextureData(const void* data, const Texture* texture, u32 mip = 0, u32 arraySlice = 0);
        Ref<ReadbackBuffer> ReadbackTextureData(const Texture* texture, u32 mip = 0, u32 arraySlice = 0);
        void CopyTexture(const Texture* srcTexture, const Texture* dstTexture, u32 subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
        void DrawIndexed(u32 indexCount, u32 instanceCount = 1, u32 startIndex = 0, u32 startVertex = 0, u32 startInstance = 0);
        void Dispatch(u32 threadCountX, u32 threadCountY, u32 threadCountZ);
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
        const GraphicsPipeline*            m_CurrentGraphicsPipeline = nullptr;
        const ComputePipeline*             m_CurrentComputePipeline = nullptr;
        bool                               m_IsRecording = false;
    };
}