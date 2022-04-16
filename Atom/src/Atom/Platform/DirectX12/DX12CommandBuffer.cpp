#include "atompch.h"

#if defined(ATOM_PLATFORM_WINDOWS)

#include "DX12CommandBuffer.h"
#include "DX12Device.h"
#include "DX12Texture.h"
#include "DX12TextureView.h"
#include "DX12SwapChain.h"
#include "DX12GraphicsPipeline.h"
#include "DX12Framebuffer.h"

#include "Atom/Core/Application.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    DX12CommandBuffer::DX12CommandBuffer()
    {
        auto d3dDevice = Renderer::GetDevice().As<DX12Device>()->GetD3DDevice();
        u32 framesInFlight = Renderer::GetFramesInFlight();

        // Create command allocators for each frame
        m_Allocators.resize(framesInFlight);
        for (u32 i = 0; i < framesInFlight; i++)
        {
            DXCall(d3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_Allocators[i])));
        }

        // Create command list
        DXCall(d3dDevice->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&m_CommandList)));
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    DX12CommandBuffer::~DX12CommandBuffer()
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void DX12CommandBuffer::Begin()
    {
        u32 frameIndex = Renderer::GetCurrentFrameIndex();
        DXCall(m_Allocators[frameIndex]->Reset());
        DXCall(m_CommandList->Reset(m_Allocators[frameIndex].Get(), NULL));
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void DX12CommandBuffer::TransitionResource(const Ref<Texture>& texture, ResourceState beforeState, ResourceState afterState)
    {
        DX12Texture* dx12Texture = texture->As<DX12Texture>();

        auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(dx12Texture->GetD3DResource().Get(), 
                                                            Utils::AtomResourceStateToD3D12(beforeState), 
                                                            Utils::AtomResourceStateToD3D12(afterState));
        m_CommandList->ResourceBarrier(1, &barrier);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void DX12CommandBuffer::BeginRenderPass(const Ref<Framebuffer>& framebuffer, bool clear)
    {
        auto dx12FrameBuffer = framebuffer->As<DX12Framebuffer>();

        if (clear)
        {
            for (u32 i = 0; i < AttachmentPoint::NumColorAttachments; i++)
            {
                auto renderTarget = dx12FrameBuffer->GetRTV((AttachmentPoint)i);
                if (renderTarget)
                {
                    auto dx12RenderTarget = renderTarget->As<DX12TextureViewRT>();
                    m_CommandList->ClearRenderTargetView(dx12RenderTarget->GetDescriptor().GetCPUHandle(), dx12FrameBuffer->GetClearColor(), 0, nullptr);
                }
            }

            auto depthBuffer = dx12FrameBuffer->GetDSV();
            if (depthBuffer)
            {
                auto dx12DepthBuffer = depthBuffer->As<DX12TextureViewDS>();

                D3D12_CLEAR_FLAGS clearFlags = D3D12_CLEAR_FLAG_DEPTH;
                if (dx12FrameBuffer->GetAttachmnt(AttachmentPoint::DepthStencil)->GetFormat() == TextureFormat::Depth24Stencil8)
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

        auto depthBuffer = dx12FrameBuffer->GetDSV();
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
    void DX12CommandBuffer::SetGraphicsPipeline(const Ref<GraphicsPipeline>& pipeline)
    {
        auto dx12Pipeline = pipeline->As<DX12GraphicsPipeline>();

        m_CommandList->SetGraphicsRootSignature(dx12Pipeline->GetD3DDescription().pRootSignature);
        m_CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_CommandList->SetPipelineState(dx12Pipeline->GetD3DPipeline().Get());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void DX12CommandBuffer::Draw(u32 count)
    {
        m_CommandList->DrawInstanced(count, 1, 0, 0);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void DX12CommandBuffer::End()
    {
        DXCall(m_CommandList->Close());
    }
}

#endif // ATOM_PLATFORM_WINDOWS
