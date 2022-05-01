#include "atompch.h"

#include "Atom/Core/DirectX12/DirectX12Utils.h"
#include "Atom/Core/Application.h"

#include "CommandBuffer.h"
#include "Device.h"
#include "Texture.h"
#include "TextureView.h"
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
        auto d3dDevice = Renderer::GetDevice()->GetD3DDevice();

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
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandBuffer::BeginRenderPass(const Framebuffer* framebuffer, bool clear)
    {
        // Transition all resources to render target state
        for (u32 i = 0; i < AttachmentPoint::NumColorAttachments; i++)
        {
            auto renderTarget = framebuffer->GetRTV((AttachmentPoint)i);
            if (renderTarget)
            {
                m_ResourceStateTracker.AddTransition(renderTarget->GetTextureResource()->GetD3DResource().Get(), D3D12_RESOURCE_STATE_RENDER_TARGET);
            }
        }

        auto depthBuffer = framebuffer->GetDSV();
        if (depthBuffer)
        {
            m_ResourceStateTracker.AddTransition(depthBuffer->GetTextureResource()->GetD3DResource().Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE);
        }

        if (clear)
        {
            for (u32 i = 0; i < AttachmentPoint::NumColorAttachments; i++)
            {
                auto renderTarget = framebuffer->GetRTV((AttachmentPoint)i);
                if (renderTarget)
                {
                    m_CommandList->ClearRenderTargetView(renderTarget->GetDescriptor(), glm::value_ptr(framebuffer->GetClearColor()), 0, nullptr);
                }
            }

            auto depthBuffer = framebuffer->GetDSV();
            if (depthBuffer)
            {
                D3D12_CLEAR_FLAGS clearFlags = D3D12_CLEAR_FLAG_DEPTH;
                if (framebuffer->GetAttachment(AttachmentPoint::DepthStencil)->GetFormat() == TextureFormat::Depth24Stencil8)
                {
                    clearFlags |= D3D12_CLEAR_FLAG_STENCIL;
                }

                m_CommandList->ClearDepthStencilView(depthBuffer->GetDescriptor(), clearFlags, 0.0f, 0xff, 0, nullptr);
            }
        }

        // Set color attachments
        Vector<D3D12_CPU_DESCRIPTOR_HANDLE> rtvHandles;
        rtvHandles.reserve(AttachmentPoint::NumColorAttachments);

        for (u32 i = 0; i < AttachmentPoint::NumColorAttachments; i++)
        {
            auto renderTarget = framebuffer->GetRTV((AttachmentPoint)i);
            if (renderTarget)
            {
                rtvHandles.push_back(renderTarget->GetDescriptor());
            }
        }

        // Set depth buffer
        D3D12_CPU_DESCRIPTOR_HANDLE* dsvHandle = nullptr;

        if (depthBuffer)
        {
            dsvHandle = &depthBuffer->GetDescriptor();
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
            auto colorAttachment = framebuffer->GetAttachment((AttachmentPoint)i);
            if (colorAttachment)
            {
                m_ResourceStateTracker.AddTransition(colorAttachment->GetD3DResource().Get(), D3D12_RESOURCE_STATE_COMMON);
            }
        }

        auto depthBuffer = framebuffer->GetAttachment(AttachmentPoint::DepthStencil);
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
        m_CommandList->IASetVertexBuffers(0, 1, &vertexBuffer->GetBufferView());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void CommandBuffer::SetIndexBuffer(const IndexBuffer* indexBuffer)
    {
        m_ResourceStateTracker.AddTransition(indexBuffer->GetD3DResource().Get(), D3D12_RESOURCE_STATE_INDEX_BUFFER);
        m_CommandList->IASetIndexBuffer(&indexBuffer->GetBufferView());
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
    }
}
