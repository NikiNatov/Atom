#include "atompch.h"

#include "Atom/Core/DirectX12/DirectX12Utils.h"
#include "Atom/Core/Application.h"

#include "CommandBuffer.h"
#include "CommandQueue.h"
#include "Device.h"
#include "Texture.h"
#include "SwapChain.h"
#include "Pipeline.h"
#include "Framebuffer.h"
#include "Buffer.h"
#include "Renderer.h"

#include <glm\gtc\type_ptr.hpp>

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    CommandBuffer::CommandBuffer(CommandQueueType type, const char* debugName)
        : m_ResourceStateTracker(*this)
    {
        auto d3dDevice = Device::Get().GetD3DDevice();

        DXCall(d3dDevice->CreateCommandAllocator(Utils::AtomCommandQueueTypeToD3D12(type), IID_PPV_ARGS(&m_Allocator)));
        DXCall(d3dDevice->CreateCommandAllocator(Utils::AtomCommandQueueTypeToD3D12(type), IID_PPV_ARGS(&m_PendingAllocator)));

        DXCall(d3dDevice->CreateCommandList1(0, Utils::AtomCommandQueueTypeToD3D12(type), D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&m_CommandList)));
        DXCall(d3dDevice->CreateCommandList1(0, Utils::AtomCommandQueueTypeToD3D12(type), D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&m_PendingCommandList)));

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

        DXCall(m_PendingAllocator->Reset());
        DXCall(m_PendingCommandList->Reset(m_PendingAllocator.Get(), NULL));

        m_UploadBuffers.clear();
        m_ResourceStateTracker.ClearStates();
        m_IsRecording = true;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandBuffer::BeginRenderPass(const Framebuffer* framebuffer, bool clear)
    {
        ATOM_ENGINE_ASSERT(m_IsRecording);

        // Set color attachments
        Vector<D3D12_CPU_DESCRIPTOR_HANDLE> rtvHandles;
        rtvHandles.reserve(AttachmentPoint::NumColorAttachments);

        for (u32 i = 0; i < AttachmentPoint::NumColorAttachments; i++)
        {
            auto colorAttachment = framebuffer->GetColorAttachment((AttachmentPoint)i);
            if (colorAttachment)
            {
                m_ResourceStateTracker.AddTransition(colorAttachment->GetD3DResource().Get(), D3D12_RESOURCE_STATE_RENDER_TARGET);

                if (clear)
                {
                    m_CommandList->ClearRenderTargetView(colorAttachment->GetRTV(), glm::value_ptr(framebuffer->GetClearColor()), 0, nullptr);
                }

                rtvHandles.push_back(colorAttachment->GetRTV());
            }
        }

        // Set depth buffer
        D3D12_CPU_DESCRIPTOR_HANDLE* dsvHandle = nullptr;
        auto depthBuffer = framebuffer->GetDepthAttachment();

        if (depthBuffer)
        {
            m_ResourceStateTracker.AddTransition(depthBuffer->GetD3DResource().Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE);

            if (clear)
            {
                D3D12_CLEAR_FLAGS clearFlags = D3D12_CLEAR_FLAG_DEPTH;
                if (depthBuffer->GetFormat() == TextureFormat::Depth24Stencil8)
                {
                    clearFlags |= D3D12_CLEAR_FLAG_STENCIL;
                }

                const DepthStencilValue& clearValue = depthBuffer->GetClearValue().DepthStencil;
                m_CommandList->ClearDepthStencilView(depthBuffer->GetDSV(), clearFlags, clearValue.DepthValue, clearValue.StencilValue, 0, nullptr);
            }

            dsvHandle = &depthBuffer->GetDSV();
        }

        m_CommandList->RSSetViewports(1, &framebuffer->GetViewport());
        m_CommandList->RSSetScissorRects(1, &framebuffer->GetScissorRect());
        m_CommandList->OMSetRenderTargets(rtvHandles.size(), rtvHandles.data(), false, dsvHandle);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandBuffer::EndRenderPass(const Framebuffer* framebuffer)
    {
        ATOM_ENGINE_ASSERT(m_IsRecording);

        for (u32 i = 0; i < AttachmentPoint::NumColorAttachments; i++)
        {
            auto colorAttachment = framebuffer->GetColorAttachment((AttachmentPoint)i);
            if (colorAttachment)
            {
                m_ResourceStateTracker.AddTransition(colorAttachment->GetD3DResource().Get(), D3D12_RESOURCE_STATE_COMMON);
            }
        }

        auto depthBuffer = framebuffer->GetDepthAttachment();
        if (depthBuffer)
        {
            m_ResourceStateTracker.AddTransition(depthBuffer->GetD3DResource().Get(), D3D12_RESOURCE_STATE_DEPTH_READ);
        }

        m_ResourceStateTracker.CommitBarriers();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandBuffer::TransitionResource(const Texture* texture, D3D12_RESOURCE_STATES state, u32 subresource)
    {
        ATOM_ENGINE_ASSERT(m_IsRecording);

        m_ResourceStateTracker.AddTransition(texture->GetD3DResource().Get(), state, subresource);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandBuffer::AddUAVBarrier(const Texture* texture)
    {
        ATOM_ENGINE_ASSERT(m_IsRecording);

        m_ResourceStateTracker.AddUAVBarrier(texture->GetD3DResource().Get());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandBuffer::CommitBarriers()
    {
        ATOM_ENGINE_ASSERT(m_IsRecording);

        m_ResourceStateTracker.CommitBarriers();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandBuffer::SetVertexBuffer(const VertexBuffer* vertexBuffer)
    {
        ATOM_ENGINE_ASSERT(m_IsRecording);

        if(!vertexBuffer->IsDynamic())
            m_ResourceStateTracker.AddTransition(vertexBuffer->GetD3DResource().Get(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

        m_CommandList->IASetVertexBuffers(0, 1, &vertexBuffer->GetVertexBufferView());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandBuffer::SetIndexBuffer(const IndexBuffer* indexBuffer)
    {
        ATOM_ENGINE_ASSERT(m_IsRecording);

        if (!indexBuffer->IsDynamic())
            m_ResourceStateTracker.AddTransition(indexBuffer->GetD3DResource().Get(), D3D12_RESOURCE_STATE_INDEX_BUFFER);

        m_CommandList->IASetIndexBuffer(&indexBuffer->GetIndexBufferView());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandBuffer::SetGraphicsPipeline(const GraphicsPipeline* pipeline)
    {
        ATOM_ENGINE_ASSERT(m_IsRecording);

        m_CommandList->SetGraphicsRootSignature(pipeline->GetD3DDescription().pRootSignature);
        m_CommandList->IASetPrimitiveTopology(Utils::D3D12TopologyTypeToD3D12Topology(pipeline->GetD3DDescription().PrimitiveTopologyType));
        m_CommandList->SetPipelineState(pipeline->GetD3DPipeline().Get());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandBuffer::SetComputePipeline(const ComputePipeline* pipeline)
    {
        ATOM_ENGINE_ASSERT(m_IsRecording);

        m_CommandList->SetComputeRootSignature(pipeline->GetD3DDescription().pRootSignature);
        m_CommandList->SetPipelineState(pipeline->GetD3DPipeline().Get());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandBuffer::SetGraphicsConstantBuffer(u32 rootParamIndex, const ConstantBuffer* constantBuffer)
    {
        ATOM_ENGINE_ASSERT(m_IsRecording);

        if (!constantBuffer->IsDynamic())
            m_ResourceStateTracker.AddTransition(constantBuffer->GetD3DResource().Get(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

        m_CommandList->SetGraphicsRootConstantBufferView(rootParamIndex, constantBuffer->GetD3DResource()->GetGPUVirtualAddress());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandBuffer::SetGraphicsStructuredBuffer(u32 rootParamIndex, const StructuredBuffer* structuredBuffer)
    {
        ATOM_ENGINE_ASSERT(m_IsRecording);

        if (!structuredBuffer->IsDynamic())
            m_ResourceStateTracker.AddTransition(structuredBuffer->GetD3DResource().Get(), D3D12_RESOURCE_STATE_GENERIC_READ);

        m_CommandList->SetGraphicsRootShaderResourceView(rootParamIndex, structuredBuffer->GetD3DResource()->GetGPUVirtualAddress());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandBuffer::SetComputeConstantBuffer(u32 rootParamIndex, const ConstantBuffer* constantBuffer)
    {
        ATOM_ENGINE_ASSERT(m_IsRecording);

        if (!constantBuffer->IsDynamic())
            m_ResourceStateTracker.AddTransition(constantBuffer->GetD3DResource().Get(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

        m_CommandList->SetComputeRootConstantBufferView(rootParamIndex, constantBuffer->GetD3DResource()->GetGPUVirtualAddress());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandBuffer::SetComputeStructuredBuffer(u32 rootParamIndex, const StructuredBuffer* structuredBuffer)
    {
        ATOM_ENGINE_ASSERT(m_IsRecording);

        if (!structuredBuffer->IsDynamic())
            m_ResourceStateTracker.AddTransition(structuredBuffer->GetD3DResource().Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

        m_CommandList->SetComputeRootShaderResourceView(rootParamIndex, structuredBuffer->GetD3DResource()->GetGPUVirtualAddress());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandBuffer::SetGraphicsRootConstants(u32 rootParamIndex, const void* data, u32 numConstants)
    {
        ATOM_ENGINE_ASSERT(m_IsRecording);
        m_CommandList->SetGraphicsRoot32BitConstants(rootParamIndex, numConstants, data, 0);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandBuffer::SetComputeRootConstants(u32 rootParamIndex, const void* data, u32 numConstants)
    {
        ATOM_ENGINE_ASSERT(m_IsRecording);
        m_CommandList->SetComputeRoot32BitConstants(rootParamIndex, numConstants, data, 0);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandBuffer::SetGraphicsDescriptorTable(u32 rootParamIndex, D3D12_GPU_DESCRIPTOR_HANDLE tableStart)
    {
        ATOM_ENGINE_ASSERT(m_IsRecording);
        m_CommandList->SetGraphicsRootDescriptorTable(rootParamIndex, tableStart);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandBuffer::SetComputeDescriptorTable(u32 rootParamIndex, D3D12_GPU_DESCRIPTOR_HANDLE tableStart)
    {
        ATOM_ENGINE_ASSERT(m_IsRecording);
        m_CommandList->SetComputeRootDescriptorTable(rootParamIndex, tableStart);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandBuffer::SetDescriptorHeaps(const DescriptorHeap* resourceHeap, const DescriptorHeap* samplerHeap)
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
    void CommandBuffer::UploadBufferData(const void* data, const Buffer* buffer)
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

            m_ResourceStateTracker.AddTransition(buffer->GetD3DResource().Get(), D3D12_RESOURCE_STATE_COPY_DEST);
            m_ResourceStateTracker.CommitBarriers();
            UpdateSubresources<1>(m_CommandList.Get(), buffer->GetD3DResource().Get(), uploadBuffer.Get(), 0, 0, 1, &subresourceData);
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandBuffer::UploadTextureData(const void* data, const Texture* texture, u32 mip, u32 arraySlice)
    {
        ATOM_ENGINE_ASSERT(m_IsRecording);

        if (data)
        {
            u32 currentFrameIndex = Renderer::GetCurrentFrameIndex();
            u32 subresourceIdx = D3D12CalcSubresource(mip, arraySlice, 0, texture->GetMipLevels(), texture->GetType() == TextureType::TextureCube ? 6 : 1);
            u32 bufferSize = GetRequiredIntermediateSize(texture->GetD3DResource().Get(), subresourceIdx, 1);
            CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

            ComPtr<ID3D12Resource> uploadBuffer = nullptr;
            DXCall(Device::Get().GetD3DDevice()->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadBuffer)));

#if defined (ATOM_DEBUG)
            DXCall(uploadBuffer->SetName(L"Upload Buffer"));
#endif
            m_UploadBuffers.push_back(uploadBuffer);

            D3D12_SUBRESOURCE_DATA subresourceData = {};
            subresourceData.pData = data;
            subresourceData.RowPitch = texture->GetWidth() * Utils::GetTextureFormatSize(texture->GetFormat());
            subresourceData.SlicePitch = texture->GetHeight() * subresourceData.RowPitch;

            m_ResourceStateTracker.AddTransition(texture->GetD3DResource().Get(), D3D12_RESOURCE_STATE_COPY_DEST);
            m_ResourceStateTracker.CommitBarriers();
            UpdateSubresources<1>(m_CommandList.Get(), texture->GetD3DResource().Get(), uploadBuffer.Get(), 0, subresourceIdx, 1, &subresourceData);
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandBuffer::CopyTexture(const Texture* srcTexture, const Texture* dstTexture, u32 subresource)
    {
        ATOM_ENGINE_ASSERT(m_IsRecording);

        CD3DX12_TEXTURE_COPY_LOCATION dstLocation(dstTexture->GetD3DResource().Get(), subresource);
        CD3DX12_TEXTURE_COPY_LOCATION srcLocation(srcTexture->GetD3DResource().Get(), subresource);

        m_ResourceStateTracker.CommitBarriers();
        m_CommandList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandBuffer::DrawIndexed(u32 indexCount, u32 instanceCount, u32 startIndex, u32 startVertex, u32 startInstance)
    {
        ATOM_ENGINE_ASSERT(m_IsRecording);

        m_ResourceStateTracker.CommitBarriers();
        m_CommandList->DrawIndexedInstanced(indexCount, instanceCount, startIndex, startVertex, startInstance);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandBuffer::Dispatch(u32 threadCountX, u32 threadCountY, u32 threadCountZ)
    {
        ATOM_ENGINE_ASSERT(m_IsRecording);

        m_ResourceStateTracker.CommitBarriers();
        m_CommandList->Dispatch(threadCountX, threadCountY, threadCountZ);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandBuffer::End()
    {
        ATOM_ENGINE_ASSERT(m_IsRecording);

        m_ResourceStateTracker.CommitBarriers();
        DXCall(m_CommandList->Close());

        m_ResourceStateTracker.CommitPendingBarriers();
        m_ResourceStateTracker.UpdateGlobalStates();
        DXCall(m_PendingCommandList->Close());

        m_IsRecording = false;
    }
}
