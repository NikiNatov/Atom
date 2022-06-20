#include "atompch.h"

#include "Atom/Core/DirectX12/DirectX12Utils.h"
#include "Atom/Core/Application.h"

#include "CommandBuffer.h"
#include "Device.h"
#include "Texture.h"
#include "SwapChain.h"
#include "GraphicsPipeline.h"
#include "Framebuffer.h"
#include "Buffer.h"
#include "Renderer.h"

#include <glm\gtc\type_ptr.hpp>

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    CommandBuffer::CommandBuffer(const char* debugName)
        : m_ResourceStateTracker(*this)
    {
        auto d3dDevice = Device::Get().GetD3DDevice();

        u32 framesInFlight = Renderer::GetFramesInFlight();
        m_Allocators.resize(framesInFlight);
        m_PendingAllocators.resize(framesInFlight);
        m_UploadBuffers.resize(framesInFlight);
        for (u32 i = 0; i < framesInFlight; i++)
        {
            DXCall(d3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_Allocators[i])));
            DXCall(d3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_PendingAllocators[i])));
            m_UploadBuffers[i] = nullptr;
        }

        DXCall(d3dDevice->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&m_CommandList)));
        DXCall(d3dDevice->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&m_PendingCommandList)));

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
        u32 currentFrame = Renderer::GetCurrentFrameIndex();
        DXCall(m_Allocators[currentFrame]->Reset());
        DXCall(m_CommandList->Reset(m_Allocators[currentFrame].Get(), NULL));

        DXCall(m_PendingAllocators[currentFrame]->Reset());
        DXCall(m_PendingCommandList->Reset(m_PendingAllocators[currentFrame].Get(), NULL));

        m_ResourceStateTracker.ClearStates();
        m_IsRecording = true;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandBuffer::BeginRenderPass(const Framebuffer* framebuffer, bool clear)
    {
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

                m_CommandList->ClearDepthStencilView(depthBuffer->GetDSV(), clearFlags, 0.0f, 0xff, 0, nullptr);
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
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandBuffer::SetGraphicsPipeline(const GraphicsPipeline* pipeline)
    {
        m_CommandList->SetGraphicsRootSignature(pipeline->GetD3DDescription().pRootSignature);
        m_CommandList->IASetPrimitiveTopology(Utils::D3D12TopologyTypeToD3D12Topology(pipeline->GetD3DDescription().PrimitiveTopologyType));
        m_CommandList->SetPipelineState(pipeline->GetD3DPipeline().Get());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandBuffer::SetVertexBuffer(const VertexBuffer* vertexBuffer)
    {
        m_ResourceStateTracker.AddTransition(vertexBuffer->GetD3DResource().Get(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
        m_CommandList->IASetVertexBuffers(0, 1, &vertexBuffer->GetVertexBufferView());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandBuffer::SetIndexBuffer(const IndexBuffer* indexBuffer)
    {
        m_ResourceStateTracker.AddTransition(indexBuffer->GetD3DResource().Get(), D3D12_RESOURCE_STATE_INDEX_BUFFER);
        m_CommandList->IASetIndexBuffer(&indexBuffer->GetIndexBufferView());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandBuffer::UploadBufferData(const void* data, u32 size, const Buffer* buffer)
    {
        ATOM_ENGINE_ASSERT(m_IsRecording);

        if (data)
        {
            CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(buffer->GetSize());

            DXCall(Device::Get().GetD3DDevice()->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_UploadBuffers[Renderer::GetCurrentFrameIndex()])));

#if defined (ATOM_DEBUG)
            DXCall(m_UploadBuffers[Renderer::GetCurrentFrameIndex()]->SetName(L"Upload Buffer"));
#endif

            D3D12_SUBRESOURCE_DATA subresourceData = {};
            subresourceData.pData = data;
            subresourceData.RowPitch = size;
            subresourceData.SlicePitch = subresourceData.RowPitch;

            m_ResourceStateTracker.AddTransition(buffer->GetD3DResource().Get(), D3D12_RESOURCE_STATE_COPY_DEST);
            UpdateSubresources<1>(m_CommandList.Get(), buffer->GetD3DResource().Get(), m_UploadBuffers[Renderer::GetCurrentFrameIndex()].Get(), 0, 0, 1, &subresourceData);
            m_ResourceStateTracker.AddTransition(buffer->GetD3DResource().Get(), Utils::GetInitialStateFromBufferType(buffer->GetType()));
            m_ResourceStateTracker.CommitBarriers();
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandBuffer::SetConstantBuffer(u32 slot, const ConstantBuffer* constantBuffer)
    {
        m_CommandList->SetGraphicsRootConstantBufferView(slot, constantBuffer->GetD3DResource()->GetGPUVirtualAddress());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandBuffer::DrawIndexed(u32 indexCount)
    {
        m_ResourceStateTracker.CommitBarriers();
        m_CommandList->DrawIndexedInstanced(indexCount, 1, 0, 0, 0);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandBuffer::End()
    {
        m_ResourceStateTracker.CommitBarriers();
        DXCall(m_CommandList->Close());

        m_ResourceStateTracker.CommitPendingBarriers();
        m_ResourceStateTracker.UpdateGlobalStates();
        DXCall(m_PendingCommandList->Close());

        m_IsRecording = false;
    }
}
