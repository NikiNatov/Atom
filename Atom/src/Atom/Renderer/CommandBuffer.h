#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Core/DirectX12/DirectX12.h"

#include "Atom/Renderer/ShaderLayout.h"
#include "Atom/Renderer/ResourceStateTracker.h"
#include "Atom/Renderer/DescriptorHeap.h"

namespace Atom
{
    class Texture;
    class GraphicsPipeline;
    class ComputePipeline;
    class Buffer;
    class VertexBuffer;
    class IndexBuffer;
    class ConstantBuffer;
    class StructuredBuffer;
    class ReadbackBuffer;
    class GPUDescriptorHeap;
    class ComputeShader;
    class ResourceBarrier;
    class RenderSurface;
    enum class CommandQueueType;

    class CommandBuffer
    {
    public:
        CommandBuffer(CommandQueueType type, const char* debugName = "Unnamed Command Buffer");
        ~CommandBuffer();

        void Begin();
        void BeginRenderPass(const Vector<RenderSurface*> renderTargets, bool clear = false);
        void TransitionResource(const HWResource* resource, ResourceState state, u32 subresource = UINT32_MAX);
        void AddUAVBarrier(const HWResource* resource);
        void CommitBarriers();
        void SetVertexBuffer(const VertexBuffer* vertexBuffer);
        void SetIndexBuffer(const IndexBuffer* indexBuffer);
        void SetGraphicsPipeline(const GraphicsPipeline* pipeline);
        void SetComputePipeline(const ComputePipeline* pipeline);
        void SetGraphicsConstants(ShaderBindPoint bindPoint, const ConstantBuffer* constantBuffer);
        void SetComputeConstants(ShaderBindPoint bindPoint, const ConstantBuffer* constantBuffer);
        void SetGraphicsConstants(ShaderBindPoint bindPoint, const void* data, u32 numConstants);
        void SetComputeConstants(ShaderBindPoint bindPoint, const void* data, u32 numConstants);
        void SetGraphicsDescriptorTables(ShaderBindPoint bindPoint, const DescriptorAllocation& resourceTable, const DescriptorAllocation& samplerTable = DescriptorAllocation());
        void SetComputeDescriptorTables(ShaderBindPoint bindPoint, const DescriptorAllocation& resourceTable, const DescriptorAllocation& samplerTable = DescriptorAllocation());
        void SetDescriptorHeaps(const GPUDescriptorHeap* resourceHeap, const GPUDescriptorHeap* samplerHeap);
        void CopyTexture(const Texture* srcTexture, const Texture* dstTexture, u32 subresource = UINT32_MAX);
        void CopyTexture(const Texture* srcTexture, const Buffer* dstBuffer, u32 subresource = UINT32_MAX);
        void DrawIndexed(u32 indexCount, u32 instanceCount = 1, u32 startIndex = 0, u32 startVertex = 0, u32 startInstance = 0);
        void Dispatch(u32 threadCountX, u32 threadCountY, u32 threadCountZ);
        void End();

        Ref<CommandBuffer> GetPendingBarriersCommandBuffer();

        inline CommandQueueType GetQueueType() const { return m_QueueType; }
        inline const ResourceStateTracker::ResourceStateMap& GetResourceStates() const { return m_ResourceStates; }
        inline ComPtr<ID3D12GraphicsCommandList6> GetCommandList() const { return m_CommandList; }
    private:
        CommandQueueType                       m_QueueType;
        ComPtr<ID3D12CommandAllocator>         m_Allocator;
        ComPtr<ID3D12GraphicsCommandList6>     m_CommandList;

        Vector<Ref<TransitionBarrier>>         m_PendingBarriers;
        Vector<Ref<ResourceBarrier>>           m_ResourceBarriers;
        ResourceStateTracker::ResourceStateMap m_ResourceStates;

        const GraphicsPipeline*                m_CurrentGraphicsPipeline = nullptr;
        const ComputePipeline*                 m_CurrentComputePipeline = nullptr;
        bool                                   m_IsRecording = false;
    };
}