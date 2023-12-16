#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Core/DirectX12/DirectX12.h"

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
    class ShaderInputGroupLayout;
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
        void SetDescriptorHeaps(const GPUDescriptorHeap* resourceHeap, const GPUDescriptorHeap* samplerHeap);
        void CopyTexture(const Texture* srcTexture, const Texture* dstTexture, u32 subresource = UINT32_MAX);
        void CopyTexture(const Texture* srcTexture, const Buffer* dstBuffer, u32 subresource = UINT32_MAX);
        void DrawIndexed(u32 indexCount, u32 instanceCount = 1, u32 startIndex = 0, u32 startVertex = 0, u32 startInstance = 0);
        void Dispatch(u32 threadCountX, u32 threadCountY, u32 threadCountZ);
        void End();

        template<typename SIGType>
        void SetGraphicsSIG(const SIGType& sig)
        {
            ATOM_ENGINE_ASSERT(m_IsRecording);
            const ShaderInputGroupStorage& storage = sig.GetStorage();

            if (SIGType::BindPointType::ConstantBufferRootIndex != UINT32_MAX)
                m_CommandList->SetGraphicsRootConstantBufferView(SIGType::BindPointType::ConstantBufferRootIndex, storage.GetConstantBuffer()->GetD3DResource()->GetGPUVirtualAddress());

            if (SIGType::BindPointType::ResourceTableRootIndex != UINT32_MAX)
                m_CommandList->SetGraphicsRootDescriptorTable(SIGType::BindPointType::ResourceTableRootIndex, storage.GetResourceTable().GetBaseGpuDescriptor());

            if (SIGType::BindPointType::SamplerTableRootIndex != UINT32_MAX)
                m_CommandList->SetGraphicsRootDescriptorTable(SIGType::BindPointType::SamplerTableRootIndex, storage.GetSamplerTable().GetBaseGpuDescriptor());
        }

        template<typename SIGType>
        void SetComputeSIG(const SIGType& sig)
        {
            ATOM_ENGINE_ASSERT(m_IsRecording);
            const ShaderInputGroupStorage& storage = sig.GetStorage();

            if (SIGType::BindPointType::ConstantBufferRootIndex != UINT32_MAX)
                m_CommandList->SetComputeRootConstantBufferView(SIGType::BindPointType::ConstantBufferRootIndex, storage.GetConstantBuffer()->GetD3DResource()->GetGPUVirtualAddress());

            if (SIGType::BindPointType::ResourceTableRootIndex != UINT32_MAX)
                m_CommandList->SetComputeRootDescriptorTable(SIGType::BindPointType::ResourceTableRootIndex, storage.GetResourceTable().GetBaseGpuDescriptor());

            if (SIGType::BindPointType::SamplerTableRootIndex != UINT32_MAX)
                m_CommandList->SetComputeRootDescriptorTable(SIGType::BindPointType::SamplerTableRootIndex, storage.GetSamplerTable().GetBaseGpuDescriptor());
        }

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