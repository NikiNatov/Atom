#include "atompch.h"

#if defined(ATOM_PLATFORM_WINDOWS)

#include "DX12CommandBuffer.h"
#include "DX12Device.h"
#include "DX12Texture.h"
#include "DX12TextureView.h"
#include "DX12SwapChain.h"
#include "DX12GraphicsPipeline.h"
#include "DX12Framebuffer.h"
#include "DX12Buffer.h"

#include "Atom/Core/Application.h"

#include <glm\gtc\type_ptr.hpp>

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    DX12CommandBuffer::DX12CommandBuffer(const char* debugName)
        : m_ResourceStateTracker(*this)
    {
        auto d3dDevice = Renderer::GetDevice().As<DX12Device>()->GetD3DDevice();

        u32 framesInFlight = Renderer::GetFramesInFlight();
        m_Allocators.resize(framesInFlight);
        m_PendingAllocators.resize(framesInFlight);
        for (u32 i = 0; i < framesInFlight; i++)
        {
            DXCall(d3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_Allocators[i])));
            DXCall(d3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_PendingAllocators[i])));
        }
        
        DXCall(d3dDevice->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&m_CommandList)));
        DXCall(d3dDevice->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&m_PendingCommandList)));

#if defined (ATOM_DEBUG)
        String name = debugName;
        DXCall(m_CommandList->SetName(STRING_TO_WSTRING(name).c_str()));
#endif
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    DX12CommandBuffer::~DX12CommandBuffer()
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void DX12CommandBuffer::Begin()
    {
        u32 currentFrame = Renderer::GetCurrentFrameIndex();
        DXCall(m_Allocators[currentFrame]->Reset());
        DXCall(m_CommandList->Reset(m_Allocators[currentFrame].Get(), NULL));

        DXCall(m_PendingAllocators[currentFrame]->Reset());
        DXCall(m_PendingCommandList->Reset(m_PendingAllocators[currentFrame].Get(), NULL));

        m_ResourceStateTracker.ClearStates();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void DX12CommandBuffer::BeginRenderPass(const Framebuffer* framebuffer, bool clear)
    {
        auto dx12FrameBuffer = framebuffer->As<DX12Framebuffer>();

        // Transition all resources to render target state
        for (u32 i = 0; i < AttachmentPoint::NumColorAttachments; i++)
        {
            auto renderTarget = dx12FrameBuffer->GetRTV((AttachmentPoint)i);
            if (renderTarget)
            {
                auto dx12RenderTarget = renderTarget->As<DX12TextureViewRT>();
                m_ResourceStateTracker.AddTransition(dx12RenderTarget->GetTextureResource()->As<DX12Texture>()->GetD3DResource().Get(), D3D12_RESOURCE_STATE_RENDER_TARGET);
            }
        }

        auto depthBuffer = dx12FrameBuffer->GetDSV();
        if (depthBuffer)
        {
            auto dx12DepthBuffer = depthBuffer->As<DX12TextureViewDS>();
            m_ResourceStateTracker.AddTransition(dx12DepthBuffer->GetTextureResource()->As<DX12Texture>()->GetD3DResource().Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE);
        }

        if (clear)
        {
            for (u32 i = 0; i < AttachmentPoint::NumColorAttachments; i++)
            {
                auto renderTarget = dx12FrameBuffer->GetRTV((AttachmentPoint)i);
                if (renderTarget)
                {
                    auto dx12RenderTarget = renderTarget->As<DX12TextureViewRT>();
                    m_CommandList->ClearRenderTargetView(dx12RenderTarget->GetDescriptor().GetCPUHandle(), glm::value_ptr(dx12FrameBuffer->GetClearColor()), 0, nullptr);
                }
            }

            auto depthBuffer = dx12FrameBuffer->GetDSV();
            if (depthBuffer)
            {
                auto dx12DepthBuffer = depthBuffer->As<DX12TextureViewDS>();

                D3D12_CLEAR_FLAGS clearFlags = D3D12_CLEAR_FLAG_DEPTH;
                if (dx12FrameBuffer->GetAttachment(AttachmentPoint::DepthStencil)->GetFormat() == TextureFormat::Depth24Stencil8)
                {
                    clearFlags |= D3D12_CLEAR_FLAG_STENCIL;
                }

                m_CommandList->ClearDepthStencilView(dx12DepthBuffer->GetDescriptor().GetCPUHandle(), clearFlags, 0.0f, 0xff, 0, nullptr);
            }
        }

        // Set color attachments
        Vector<D3D12_CPU_DESCRIPTOR_HANDLE> rtvHandles;
        rtvHandles.reserve(AttachmentPoint::NumColorAttachments);

        for (u32 i = 0; i < AttachmentPoint::NumColorAttachments; i++)
        {
            auto renderTarget = dx12FrameBuffer->GetRTV((AttachmentPoint)i);
            if (renderTarget)
            {
                auto dx12RenderTarget = renderTarget->As<DX12TextureViewRT>();
                rtvHandles.push_back(dx12RenderTarget->GetDescriptor().GetCPUHandle());
            }
        }

        // Set depth buffer
        D3D12_CPU_DESCRIPTOR_HANDLE* dsvHandle = nullptr;

        if (depthBuffer)
        {
            auto dx12DepthBuffer = depthBuffer->As<DX12TextureViewDS>();
            dsvHandle = &dx12DepthBuffer->GetDescriptor().GetCPUHandle();
        }

        m_CommandList->RSSetViewports(1, &dx12FrameBuffer->GetViewport());
        m_CommandList->RSSetScissorRects(1, &dx12FrameBuffer->GetScissorRect());
        m_CommandList->OMSetRenderTargets(rtvHandles.size(), rtvHandles.data(), false, dsvHandle);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void DX12CommandBuffer::EndRenderPass(const Framebuffer* framebuffer)
    {
        for (u32 i = 0; i < AttachmentPoint::NumColorAttachments; i++)
        {
            auto colorAttachment = framebuffer->GetAttachment((AttachmentPoint)i);
            if (colorAttachment)
            {
                m_ResourceStateTracker.AddTransition(colorAttachment->As<DX12Texture>()->GetD3DResource().Get(), D3D12_RESOURCE_STATE_COMMON);
            }
        }

        auto depthBuffer = framebuffer->GetAttachment(AttachmentPoint::DepthStencil);
        if (depthBuffer)
        {
            m_ResourceStateTracker.AddTransition(depthBuffer->As<DX12Texture>()->GetD3DResource().Get(), D3D12_RESOURCE_STATE_DEPTH_READ);
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void DX12CommandBuffer::SetGraphicsPipeline(const GraphicsPipeline* pipeline)
    {
        auto dx12Pipeline = pipeline->As<DX12GraphicsPipeline>();

        m_CommandList->SetGraphicsRootSignature(dx12Pipeline->GetD3DDescription().pRootSignature);
        m_CommandList->IASetPrimitiveTopology(Utils::D3D12TopologyTypeToD3D12Topology(dx12Pipeline->GetD3DDescription().PrimitiveTopologyType));
        m_CommandList->SetPipelineState(dx12Pipeline->GetD3DPipeline().Get());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void DX12CommandBuffer::SetVertexBuffer(const VertexBuffer* vertexBuffer)
    {
        auto dx12VertexBuffer = vertexBuffer->As<DX12VertexBuffer>();
        m_ResourceStateTracker.AddTransition(dx12VertexBuffer->GetD3DResource().Get(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
        m_CommandList->IASetVertexBuffers(0, 1, &dx12VertexBuffer->GetBufferView());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void DX12CommandBuffer::SetIndexBuffer(const IndexBuffer* indexBuffer)
    {
        auto dx12IndexBuffer = indexBuffer->As<DX12IndexBuffer>();
        m_ResourceStateTracker.AddTransition(dx12IndexBuffer->GetD3DResource().Get(), D3D12_RESOURCE_STATE_INDEX_BUFFER);
        m_CommandList->IASetIndexBuffer(&dx12IndexBuffer->GetBufferView());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void DX12CommandBuffer::DrawIndexed(u32 indexCount)
    {
        m_ResourceStateTracker.CommitBarriers();
        m_CommandList->DrawIndexedInstanced(indexCount, 1, 0, 0, 0);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void DX12CommandBuffer::End()
    {
        m_ResourceStateTracker.CommitBarriers();
        DXCall(m_CommandList->Close());

        m_ResourceStateTracker.CommitPendingBarriers();
        m_ResourceStateTracker.UpdateGlobalStates();
        DXCall(m_PendingCommandList->Close());
    }
}

#endif // ATOM_PLATFORM_WINDOWS
