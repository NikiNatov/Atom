#include "atompch.h"

#include "Atom/Core/DirectX12/DirectX12Utils.h"
#include "Atom/Core/Application.h"

#include "Atom/Renderer/CommandBuffer.h"
#include "Atom/Renderer/CommandQueue.h"
#include "Atom/Renderer/Device.h"
#include "Atom/Renderer/Texture.h"
#include "Atom/Renderer/SwapChain.h"
#include "Atom/Renderer/Pipeline.h"
#include "Atom/Renderer/Framebuffer.h"
#include "Atom/Renderer/Buffer.h"
#include "Atom/Renderer/Renderer.h"

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

        Vector<D3D12_CPU_DESCRIPTOR_HANDLE> rtvHandles;
        rtvHandles.reserve(AttachmentPoint::NumColorAttachments);

        D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle{ 0 };

        for (u32 i = 0; i < AttachmentPoint::NumAttachments; i++)
        {
            if (auto attachment = framebuffer->GetAttachment((AttachmentPoint)i))
            {
                bool isDepth = i == AttachmentPoint::Depth;

                if (i == AttachmentPoint::Depth)
                {
                    m_ResourceStateTracker.AddTransition(attachment->GetTexture()->GetD3DResource().Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE);

                    if (clear)
                    {
                        D3D12_CLEAR_FLAGS clearFlags = D3D12_CLEAR_FLAG_DEPTH;

                        if (attachment->GetFormat() == TextureFormat::Depth24Stencil8)
                            clearFlags |= D3D12_CLEAR_FLAG_STENCIL;

                        const auto& clearVal = attachment->GetClearValue().DepthStencil;
                        m_CommandList->ClearDepthStencilView(attachment->GetDSV()->GetDescriptor(), clearFlags, clearVal.DepthValue, clearVal.StencilValue, 0, nullptr);
                    }

                    dsvHandle = attachment->GetDSV()->GetDescriptor();
                }
                else
                {
                    m_ResourceStateTracker.AddTransition(attachment->GetTexture()->GetD3DResource().Get(), D3D12_RESOURCE_STATE_RENDER_TARGET);

                    if (clear)
                        m_CommandList->ClearRenderTargetView(attachment->GetRTV()->GetDescriptor(), glm::value_ptr(attachment->GetClearValue().Color), 0, nullptr);

                    rtvHandles.push_back(attachment->GetRTV()->GetDescriptor());
                }
            }
        }

        m_CommandList->RSSetViewports(1, &framebuffer->GetViewport());
        m_CommandList->RSSetScissorRects(1, &framebuffer->GetScissorRect());
        m_CommandList->OMSetRenderTargets(rtvHandles.size(), rtvHandles.data(), false, dsvHandle.ptr != 0 ? &dsvHandle : nullptr);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandBuffer::EndRenderPass(const Framebuffer* framebuffer)
    {
        ATOM_ENGINE_ASSERT(m_IsRecording);

        for (u32 i = 0; i < AttachmentPoint::NumAttachments; i++)
        {
            bool isDepth = i == AttachmentPoint::Depth;
            if(auto attachment = framebuffer->GetAttachment((AttachmentPoint)i))
                m_ResourceStateTracker.AddTransition(attachment->GetTexture()->GetD3DResource().Get(), isDepth ? D3D12_RESOURCE_STATE_DEPTH_READ : D3D12_RESOURCE_STATE_COMMON);
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

        if (!constantBuffer->IsDynamic())
            m_ResourceStateTracker.AddTransition(constantBuffer->GetD3DResource().Get(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

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

        if (!constantBuffer->IsDynamic())
            m_ResourceStateTracker.AddTransition(constantBuffer->GetD3DResource().Get(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

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
    void CommandBuffer::SetGraphicsDescriptorTables(ShaderBindPoint bindPoint, D3D12_GPU_DESCRIPTOR_HANDLE resourceTable, D3D12_GPU_DESCRIPTOR_HANDLE samplerTable)
    {
        ATOM_ENGINE_ASSERT(m_IsRecording);
        ATOM_ENGINE_ASSERT(m_CurrentGraphicsPipeline, "No graphics pipeline is bound");

        const auto& resourceDescriptorTable = m_CurrentGraphicsPipeline->GetShader()->GetShaderLayout().GetResourceDescriptorTable(bindPoint);
        ATOM_ENGINE_ASSERT(resourceDescriptorTable.RootParameterIndex != UINT32_MAX);

        m_CommandList->SetGraphicsRootDescriptorTable(resourceDescriptorTable.RootParameterIndex, resourceTable);

        if (samplerTable.ptr != 0)
        {
            const auto& samplerDescriptorTable = m_CurrentGraphicsPipeline->GetShader()->GetShaderLayout().GetSamplerDescriptorTable(bindPoint);
            ATOM_ENGINE_ASSERT(samplerDescriptorTable.RootParameterIndex != UINT32_MAX);

            m_CommandList->SetGraphicsRootDescriptorTable(samplerDescriptorTable.RootParameterIndex, samplerTable);
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandBuffer::SetComputeDescriptorTables(ShaderBindPoint bindPoint, D3D12_GPU_DESCRIPTOR_HANDLE resourceTable, D3D12_GPU_DESCRIPTOR_HANDLE samplerTable)
    {
        ATOM_ENGINE_ASSERT(m_IsRecording);
        ATOM_ENGINE_ASSERT(m_CurrentComputePipeline, "No compute pipeline is bound");
        
        const auto& resourceDescriptorTable = m_CurrentComputePipeline->GetComputeShader()->GetShaderLayout().GetResourceDescriptorTable(bindPoint);
        ATOM_ENGINE_ASSERT(resourceDescriptorTable.RootParameterIndex != UINT32_MAX);

        m_CommandList->SetComputeRootDescriptorTable(resourceDescriptorTable.RootParameterIndex, resourceTable);

        if (samplerTable.ptr != 0)
        {
            const auto& samplerDescriptorTable = m_CurrentComputePipeline->GetComputeShader()->GetShaderLayout().GetSamplerDescriptorTable(bindPoint);
            ATOM_ENGINE_ASSERT(samplerDescriptorTable.RootParameterIndex != UINT32_MAX);

            m_CommandList->SetComputeRootDescriptorTable(samplerDescriptorTable.RootParameterIndex, samplerTable);
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

            m_ResourceStateTracker.AddTransition(texture->GetD3DResource().Get(), D3D12_RESOURCE_STATE_COPY_DEST);
            m_ResourceStateTracker.CommitBarriers();
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

        m_ResourceStateTracker.AddTransition(texture->GetD3DResource().Get(), D3D12_RESOURCE_STATE_COPY_SOURCE);
        m_ResourceStateTracker.CommitBarriers();

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

        m_ResourceStateTracker.AddTransition(srcTexture->GetD3DResource().Get(), D3D12_RESOURCE_STATE_COPY_SOURCE);
        m_ResourceStateTracker.AddTransition(dstTexture->GetD3DResource().Get(), D3D12_RESOURCE_STATE_COPY_DEST);

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
