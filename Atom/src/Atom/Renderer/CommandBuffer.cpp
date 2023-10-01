#include "atompch.h"

#include "Atom/Core/DirectX12/DirectX12Utils.h"
#include "Atom/Core/Application.h"

#include "Atom/Renderer/CommandBuffer.h"
#include "Atom/Renderer/CommandQueue.h"
#include "Atom/Renderer/Device.h"
#include "Atom/Renderer/Texture.h"
#include "Atom/Renderer/SwapChain.h"
#include "Atom/Renderer/Pipeline.h"
#include "Atom/Renderer/Buffer.h"
#include "Atom/Renderer/Renderer.h"
#include "Atom/Renderer/ResourceBarrier.h"
#include "Atom/Renderer/RenderSurface.h"

#include <glm\gtc\type_ptr.hpp>

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    CommandBuffer::CommandBuffer(CommandQueueType type, const char* debugName)
        : m_QueueType(type)
    {
        auto d3dDevice = Device::Get().GetD3DDevice();

        DXCall(d3dDevice->CreateCommandAllocator(Utils::AtomCommandQueueTypeToD3D12(type), IID_PPV_ARGS(&m_Allocator)));
        DXCall(d3dDevice->CreateCommandList1(0, Utils::AtomCommandQueueTypeToD3D12(type), D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&m_CommandList)));

#if defined (ATOM_DEBUG)
        String name = debugName;
        DXCall(m_CommandList->SetName(STRING_TO_WSTRING(name).c_str()));
#endif
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    CommandBuffer::~CommandBuffer()
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandBuffer::Begin()
    {
        DXCall(m_Allocator->Reset());
        DXCall(m_CommandList->Reset(m_Allocator.Get(), NULL));

        m_PendingBarriers.clear();
        m_ResourceBarriers.clear();
        m_ResourceStates.clear();

        m_UploadBuffers.clear();
        m_IsRecording = true;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandBuffer::BeginRenderPass(const Vector<RenderSurface*> renderTargets, bool clear)
    {
        ATOM_ENGINE_ASSERT(m_IsRecording);

        if (renderTargets.empty())
            return;

        Vector<D3D12_CPU_DESCRIPTOR_HANDLE> rtvHandles;
        rtvHandles.reserve(AttachmentPoint::NumColorAttachments);

        D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle{ 0 };

        for (const RenderSurface* renderTarget : renderTargets)
        {
            if (renderTarget)
            {
                bool isDepth = renderTarget->GetFormat() == TextureFormat::Depth24Stencil8 || renderTarget->GetFormat() == TextureFormat::Depth32;

                if (isDepth)
                {
                    if (clear)
                    {
                        D3D12_CLEAR_FLAGS clearFlags = D3D12_CLEAR_FLAG_DEPTH;

                        if (renderTarget->GetFormat() == TextureFormat::Depth24Stencil8)
                            clearFlags |= D3D12_CLEAR_FLAG_STENCIL;

                        const auto& clearVal = renderTarget->GetClearValue().DepthStencil;
                        m_CommandList->ClearDepthStencilView(renderTarget->GetDSV()->GetDescriptor(), clearFlags, clearVal.DepthValue, clearVal.StencilValue, 0, nullptr);
                    }

                    dsvHandle = renderTarget->GetDSV()->GetDescriptor();
                }
                else
                {
                    if (clear)
                        m_CommandList->ClearRenderTargetView(renderTarget->GetRTV()->GetDescriptor(), glm::value_ptr(renderTarget->GetClearValue().Color), 0, nullptr);

                    rtvHandles.push_back(renderTarget->GetRTV()->GetDescriptor());
                }
            }
        }

        D3D12_VIEWPORT viewport = {};
        viewport.TopLeftX = 0;
        viewport.TopLeftY = 0;
        viewport.Width = renderTargets[0]->GetWidth();
        viewport.Height = renderTargets[0]->GetHeight();
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;

        D3D12_RECT scissorRect = { 0, 0, (s32)renderTargets[0]->GetWidth(), (s32)renderTargets[0]->GetHeight() };

        m_CommandList->RSSetViewports(1, &viewport);
        m_CommandList->RSSetScissorRects(1, &scissorRect);
        m_CommandList->OMSetRenderTargets(rtvHandles.size(), rtvHandles.data(), false, dsvHandle.ptr != 0 ? &dsvHandle : nullptr);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandBuffer::TransitionResource(const HWResource* resource, ResourceState state, u32 subresource)
    {
        ATOM_ENGINE_ASSERT(m_IsRecording);

        Ref<TransitionBarrier> barrier = CreateRef<TransitionBarrier>(resource, ResourceState::Common, state, subresource);

        auto stateEntry = m_ResourceStates.find(resource);
        if (stateEntry != m_ResourceStates.end())
        {
            // Resource was already used on that command list so get the current state
            ResourceStateTracker::TrackedResourceState& currentTrackedState = stateEntry->second;

            if (subresource == UINT32_MAX && !currentTrackedState.GetSubresourceStates().empty())
            {
                for (auto& [subresource, beforeState] : currentTrackedState.GetSubresourceStates())
                {
                    if (beforeState != state)
                    {
                        Ref<TransitionBarrier> subresourceBarrier = CreateRef<TransitionBarrier>(*barrier);
                        subresourceBarrier->SetBeforeState(beforeState);
                        subresourceBarrier->SetSubresource(subresource);
                        m_ResourceBarriers.push_back(subresourceBarrier);
                    }
                }
            }
            else
            {
                auto beforeState = currentTrackedState.GetState(subresource);
                if (beforeState != state)
                {
                    barrier->SetBeforeState(beforeState);
                    m_ResourceBarriers.push_back(barrier);
                }
            }
        }
        else
        {
            // Resource is being used for the first time on this command list so add transition to pending barriers
            m_PendingBarriers.push_back(barrier);
        }

        // Update the current tracked resource state
        m_ResourceStates[resource].SetState(state, subresource);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandBuffer::AddUAVBarrier(const HWResource* resource)
    {
        ATOM_ENGINE_ASSERT(m_IsRecording);

        m_ResourceBarriers.push_back(CreateRef<UAVBarrier>(resource));
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandBuffer::CommitBarriers()
    {
        ATOM_ENGINE_ASSERT(m_IsRecording);

        if (!m_ResourceBarriers.empty())
        {
            Vector<D3D12_RESOURCE_BARRIER> d3dBarriers;
            d3dBarriers.reserve(m_ResourceBarriers.size());

            for (auto& barrier : m_ResourceBarriers)
                d3dBarriers.push_back(barrier->GetD3DBarrier());

            m_CommandList->ResourceBarrier(d3dBarriers.size(), d3dBarriers.data());
        }

        m_ResourceBarriers.clear();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandBuffer::SetVertexBuffer(const VertexBuffer* vertexBuffer)
    {
        ATOM_ENGINE_ASSERT(m_IsRecording);

        m_CommandList->IASetVertexBuffers(0, 1, &vertexBuffer->GetVertexBufferView());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandBuffer::SetIndexBuffer(const IndexBuffer* indexBuffer)
    {
        ATOM_ENGINE_ASSERT(m_IsRecording);

        m_CommandList->IASetIndexBuffer(&indexBuffer->GetIndexBufferView());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandBuffer::SetGraphicsPipeline(const GraphicsPipeline* pipeline)
    {
        ATOM_ENGINE_ASSERT(m_IsRecording);

        m_CurrentGraphicsPipeline = pipeline;
        m_CurrentComputePipeline = nullptr;

        m_CommandList->SetGraphicsRootSignature(pipeline->GetD3DDescription().pRootSignature);
        m_CommandList->IASetPrimitiveTopology(Utils::D3D12TopologyTypeToD3D12Topology(pipeline->GetD3DDescription().PrimitiveTopologyType));
        m_CommandList->SetPipelineState(pipeline->GetD3DPipeline().Get());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandBuffer::SetComputePipeline(const ComputePipeline* pipeline)
    {
        ATOM_ENGINE_ASSERT(m_IsRecording);

        m_CurrentComputePipeline = pipeline;
        m_CurrentGraphicsPipeline = nullptr;

        m_CommandList->SetComputeRootSignature(pipeline->GetD3DDescription().pRootSignature);
        m_CommandList->SetPipelineState(pipeline->GetD3DPipeline().Get());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandBuffer::SetGraphicsConstants(ShaderBindPoint bindPoint, const ConstantBuffer* constantBuffer)
    {
        ATOM_ENGINE_ASSERT(m_IsRecording);
        ATOM_ENGINE_ASSERT(m_CurrentGraphicsPipeline, "No graphics pipeline is bound");
        ATOM_ENGINE_ASSERT(bindPoint != ShaderBindPoint::Material && bindPoint != ShaderBindPoint::Instance, "Material and Instance bind points use root constants");

        const auto& shaderConstants = m_CurrentGraphicsPipeline->GetShader()->GetShaderLayout().GetConstants(bindPoint);
        ATOM_ENGINE_ASSERT(shaderConstants.RootParameterIndex != UINT32_MAX);

        m_CommandList->SetGraphicsRootConstantBufferView(shaderConstants.RootParameterIndex, constantBuffer->GetD3DResource()->GetGPUVirtualAddress());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandBuffer::SetComputeConstants(ShaderBindPoint bindPoint, const ConstantBuffer* constantBuffer)
    {
        ATOM_ENGINE_ASSERT(m_IsRecording);
        ATOM_ENGINE_ASSERT(m_CurrentComputePipeline, "No compute pipeline is bound");
        ATOM_ENGINE_ASSERT(bindPoint != ShaderBindPoint::Material && bindPoint != ShaderBindPoint::Instance, "Material and Instance bind points use root constants");

        const auto& shaderConstants = m_CurrentComputePipeline->GetComputeShader()->GetShaderLayout().GetConstants(bindPoint);
        ATOM_ENGINE_ASSERT(shaderConstants.RootParameterIndex != UINT32_MAX);

        m_CommandList->SetComputeRootConstantBufferView(shaderConstants.RootParameterIndex, constantBuffer->GetD3DResource()->GetGPUVirtualAddress());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandBuffer::SetGraphicsConstants(ShaderBindPoint bindPoint, const void* data, u32 numConstants)
    {
        ATOM_ENGINE_ASSERT(m_IsRecording);
        ATOM_ENGINE_ASSERT(m_CurrentGraphicsPipeline, "No graphics pipeline is bound");
        ATOM_ENGINE_ASSERT(bindPoint == ShaderBindPoint::Material || bindPoint == ShaderBindPoint::Instance, "Only Material and Instance bind points use root constants");

        const auto& shaderConstants = m_CurrentGraphicsPipeline->GetShader()->GetShaderLayout().GetConstants(bindPoint);
        ATOM_ENGINE_ASSERT(shaderConstants.RootParameterIndex != UINT32_MAX);

        m_CommandList->SetGraphicsRoot32BitConstants(shaderConstants.RootParameterIndex, numConstants, data, 0);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandBuffer::SetComputeConstants(ShaderBindPoint bindPoint, const void* data, u32 numConstants)
    {
        ATOM_ENGINE_ASSERT(m_IsRecording);
        ATOM_ENGINE_ASSERT(m_CurrentComputePipeline, "No compute pipeline is bound");
        ATOM_ENGINE_ASSERT(bindPoint == ShaderBindPoint::Material || bindPoint == ShaderBindPoint::Instance, "Only Material and Instance bind points use root constants");

        const auto& shaderConstants = m_CurrentComputePipeline->GetComputeShader()->GetShaderLayout().GetConstants(bindPoint);
        ATOM_ENGINE_ASSERT(shaderConstants.RootParameterIndex != UINT32_MAX);

        m_CommandList->SetComputeRoot32BitConstants(shaderConstants.RootParameterIndex, numConstants, data, 0);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandBuffer::SetGraphicsDescriptorTables(ShaderBindPoint bindPoint, const DescriptorAllocation& resourceTable, const DescriptorAllocation& samplerTable)
    {
        ATOM_ENGINE_ASSERT(m_IsRecording);
        ATOM_ENGINE_ASSERT(m_CurrentGraphicsPipeline, "No graphics pipeline is bound");

        const auto& resourceDescriptorTable = m_CurrentGraphicsPipeline->GetShader()->GetShaderLayout().GetResourceDescriptorTable(bindPoint);
        ATOM_ENGINE_ASSERT(resourceDescriptorTable.RootParameterIndex != UINT32_MAX);

        m_CommandList->SetGraphicsRootDescriptorTable(resourceDescriptorTable.RootParameterIndex, resourceTable.GetBaseGpuDescriptor());

        if (samplerTable.IsValid())
        {
            const auto& samplerDescriptorTable = m_CurrentGraphicsPipeline->GetShader()->GetShaderLayout().GetSamplerDescriptorTable(bindPoint);
            ATOM_ENGINE_ASSERT(samplerDescriptorTable.RootParameterIndex != UINT32_MAX);

            m_CommandList->SetGraphicsRootDescriptorTable(samplerDescriptorTable.RootParameterIndex, samplerTable.GetBaseGpuDescriptor());
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandBuffer::SetComputeDescriptorTables(ShaderBindPoint bindPoint, const DescriptorAllocation& resourceTable, const DescriptorAllocation& samplerTable)
    {
        ATOM_ENGINE_ASSERT(m_IsRecording);
        ATOM_ENGINE_ASSERT(m_CurrentComputePipeline, "No compute pipeline is bound");
        
        const auto& resourceDescriptorTable = m_CurrentComputePipeline->GetComputeShader()->GetShaderLayout().GetResourceDescriptorTable(bindPoint);
        ATOM_ENGINE_ASSERT(resourceDescriptorTable.RootParameterIndex != UINT32_MAX);

        m_CommandList->SetComputeRootDescriptorTable(resourceDescriptorTable.RootParameterIndex, resourceTable.GetBaseGpuDescriptor());

        if (samplerTable.IsValid())
        {
            const auto& samplerDescriptorTable = m_CurrentComputePipeline->GetComputeShader()->GetShaderLayout().GetSamplerDescriptorTable(bindPoint);
            ATOM_ENGINE_ASSERT(samplerDescriptorTable.RootParameterIndex != UINT32_MAX);

            m_CommandList->SetComputeRootDescriptorTable(samplerDescriptorTable.RootParameterIndex, samplerTable.GetBaseGpuDescriptor());
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandBuffer::SetDescriptorHeaps(const GPUDescriptorHeap* resourceHeap, const GPUDescriptorHeap* samplerHeap)
    {
        ATOM_ENGINE_ASSERT(m_IsRecording);

        Vector<ID3D12DescriptorHeap*> heaps;

        if (resourceHeap)
            heaps.push_back(resourceHeap->GetD3DHeap().Get());

        if (samplerHeap)
            heaps.push_back(samplerHeap->GetD3DHeap().Get());

        if(heaps.size())
            m_CommandList->SetDescriptorHeaps(heaps.size(), heaps.data());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandBuffer::UploadBufferData(const Buffer* buffer, const void* data)
    {
        ATOM_ENGINE_ASSERT(m_IsRecording);

        if (data)
        {
            u32 currentFrameIndex = Renderer::GetCurrentFrameIndex();
            u32 bufferSize = GetRequiredIntermediateSize(buffer->GetD3DResource().Get(), 0, 1);
            CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

            ComPtr<ID3D12Resource> uploadBuffer = nullptr;
            DXCall(Device::Get().GetD3DDevice()->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadBuffer)));

#if defined (ATOM_DEBUG)
            DXCall(uploadBuffer->SetName(L"Upload Buffer"));
#endif
            m_UploadBuffers.push_back(uploadBuffer);

            D3D12_SUBRESOURCE_DATA subresourceData = {};
            subresourceData.pData = data;
            subresourceData.RowPitch = buffer->GetSize();
            subresourceData.SlicePitch = subresourceData.RowPitch;

            TransitionResource(buffer, ResourceState::CopyDestination);
            CommitBarriers();
            UpdateSubresources<1>(m_CommandList.Get(), buffer->GetD3DResource().Get(), uploadBuffer.Get(), 0, 0, 1, &subresourceData);
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandBuffer::UploadTextureData(const Texture* texture, const void* data, u32 mip, u32 arraySlice)
    {
        ATOM_ENGINE_ASSERT(m_IsRecording);

        if (data)
        {
            u32 currentFrameIndex = Renderer::GetCurrentFrameIndex();
            u32 subresourceIdx = Texture::CalculateSubresource(mip, arraySlice, texture->GetMipLevels(), texture->GetArraySize());
            u32 bufferSize = GetRequiredIntermediateSize(texture->GetD3DResource().Get(), subresourceIdx, 1);
            CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

            CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
            ComPtr<ID3D12Resource> uploadBuffer = nullptr;
            DXCall(Device::Get().GetD3DDevice()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadBuffer)));

#if defined (ATOM_DEBUG)
            DXCall(uploadBuffer->SetName(L"Upload Buffer"));
#endif
            m_UploadBuffers.push_back(uploadBuffer);

            u32 width = glm::max(texture->GetWidth() >> mip, 1u);
            u32 height = glm::max(texture->GetHeight() >> mip, 1u);

            D3D12_SUBRESOURCE_DATA subresourceData = {};
            subresourceData.pData = data;
            subresourceData.RowPitch = ((width * Utils::GetTextureFormatSize(texture->GetFormat()) + 255) / 256) * 256;
            subresourceData.SlicePitch = height * subresourceData.RowPitch;

            TransitionResource(texture, ResourceState::CopyDestination);
            CommitBarriers();
            UpdateSubresources<1>(m_CommandList.Get(), texture->GetD3DResource().Get(), uploadBuffer.Get(), 0, subresourceIdx, 1, &subresourceData);
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<ReadbackBuffer> CommandBuffer::ReadbackTextureData(const Texture* texture, u32 mip, u32 arraySlice)
    {
        ATOM_ENGINE_ASSERT(m_IsRecording);

        u32 currentFrameIndex = Renderer::GetCurrentFrameIndex();
        u32 subresourceIdx = Texture::CalculateSubresource(mip, arraySlice, texture->GetMipLevels(), texture->GetArraySize());

        BufferDescription readbackBufferDesc;
        readbackBufferDesc.ElementSize = GetRequiredIntermediateSize(texture->GetD3DResource().Get(), subresourceIdx, 1);
        readbackBufferDesc.ElementCount = 1;
        readbackBufferDesc.IsDynamic = true;

        Ref<ReadbackBuffer> readbackBuffer = CreateRef<ReadbackBuffer>(readbackBufferDesc, "Readback Buffer");

        TransitionResource(texture, ResourceState::CopySource);
        CommitBarriers();

        u32 width = glm::max(texture->GetWidth() >> mip, 1u);
        u32 height = glm::max(texture->GetHeight() >> mip, 1u);

        D3D12_PLACED_SUBRESOURCE_FOOTPRINT bufferFootprint = {};
        bufferFootprint.Footprint.Width = width;
        bufferFootprint.Footprint.Height = height;
        bufferFootprint.Footprint.Depth = 1;
        bufferFootprint.Footprint.RowPitch = ((width * Utils::GetTextureFormatSize(texture->GetFormat()) + 255) / 256) * 256;
        bufferFootprint.Footprint.Format = Utils::AtomTextureFormatToD3D12(texture->GetFormat());

        CD3DX12_TEXTURE_COPY_LOCATION copySrc(texture->GetD3DResource().Get(), subresourceIdx);
        CD3DX12_TEXTURE_COPY_LOCATION copyDst(readbackBuffer->GetD3DResource().Get(), bufferFootprint);
        m_CommandList->CopyTextureRegion(&copyDst, 0, 0, 0, &copySrc, nullptr);

        return readbackBuffer;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandBuffer::CopyTexture(const Texture* srcTexture, const Texture* dstTexture, u32 subresource)
    {
        ATOM_ENGINE_ASSERT(m_IsRecording);

        TransitionResource(srcTexture, ResourceState::CopySource);
        TransitionResource(dstTexture, ResourceState::CopyDestination);

        CD3DX12_TEXTURE_COPY_LOCATION dstLocation(dstTexture->GetD3DResource().Get(), subresource);
        CD3DX12_TEXTURE_COPY_LOCATION srcLocation(srcTexture->GetD3DResource().Get(), subresource);

        CommitBarriers();
        m_CommandList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandBuffer::DrawIndexed(u32 indexCount, u32 instanceCount, u32 startIndex, u32 startVertex, u32 startInstance)
    {
        ATOM_ENGINE_ASSERT(m_IsRecording);

        CommitBarriers();
        m_CommandList->DrawIndexedInstanced(indexCount, instanceCount, startIndex, startVertex, startInstance);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandBuffer::Dispatch(u32 threadCountX, u32 threadCountY, u32 threadCountZ)
    {
        ATOM_ENGINE_ASSERT(m_IsRecording);

        CommitBarriers();
        m_CommandList->Dispatch(threadCountX, threadCountY, threadCountZ);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandBuffer::End()
    {
        ATOM_ENGINE_ASSERT(m_IsRecording);

        // Commit any resource barriers left
        CommitBarriers();
        DXCall(m_CommandList->Close());

        m_IsRecording = false;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<CommandBuffer> CommandBuffer::GetPendingBarriersCommandBuffer()
    {
        // Resolve pending barriers first
        Vector<D3D12_RESOURCE_BARRIER> resolvedBarriers;
        resolvedBarriers.reserve(m_PendingBarriers.size());

        for (auto& pendingBarrier : m_PendingBarriers)
        {
            const auto& globalTrackedState = ResourceStateTracker::GetGlobalResourceState(pendingBarrier->GetResource());

            if (pendingBarrier->GetSubresource() == UINT32_MAX && !globalTrackedState.GetSubresourceStates().empty())
            {
                for (auto& [subresource, beforeState] : globalTrackedState.GetSubresourceStates())
                {
                    if (beforeState != pendingBarrier->GetAfterState())
                    {
                        D3D12_RESOURCE_BARRIER resolvedBarrier = pendingBarrier->GetD3DBarrier();
                        resolvedBarrier.Transition.StateBefore = Utils::AtomResourceStateToD3D12(beforeState);
                        resolvedBarrier.Transition.Subresource = subresource;

                        resolvedBarriers.push_back(resolvedBarrier);
                    }
                }
            }
            else
            {
                auto beforeState = globalTrackedState.GetState(pendingBarrier->GetSubresource());
                if (beforeState != pendingBarrier->GetAfterState())
                {
                    D3D12_RESOURCE_BARRIER resolvedBarrier = pendingBarrier->GetD3DBarrier();
                    resolvedBarrier.Transition.StateBefore = Utils::AtomResourceStateToD3D12(beforeState);

                    resolvedBarriers.push_back(resolvedBarrier);
                }
            }
        }

        m_PendingBarriers.clear();

        if (!resolvedBarriers.empty())
        {
            // Create a separate cmd list for the pending barriers
            Ref<CommandBuffer> pendingBarriersCmdBuffer = Device::Get().GetCommandQueue(m_QueueType)->GetCommandBuffer();
            pendingBarriersCmdBuffer->Begin();
            pendingBarriersCmdBuffer->m_CommandList->ResourceBarrier(resolvedBarriers.size(), resolvedBarriers.data());
            pendingBarriersCmdBuffer->End();

            return pendingBarriersCmdBuffer;
        }

        return nullptr;
    }
}
