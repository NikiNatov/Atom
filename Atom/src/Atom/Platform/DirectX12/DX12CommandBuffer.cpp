#include "atompch.h"

#if defined(ATOM_PLATFORM_WINDOWS)

#include "DX12CommandBuffer.h"
#include "DX12Device.h"
#include "DX12Texture.h"
#include "DX12TextureView.h"
#include "DX12SwapChain.h"

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
    void DX12CommandBuffer::ClearRenderTarget(const Ref<TextureViewRT>& renderTarget, const f32* color)
    {
        auto dx12RenderTarget = renderTarget->As<DX12TextureViewRT>();
        m_CommandList->ClearRenderTargetView(dx12RenderTarget->GetDescriptor().GetCPUHandle(), color, 0, nullptr);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void DX12CommandBuffer::End()
    {
        DXCall(m_CommandList->Close());
    }
}

#endif // ATOM_PLATFORM_WINDOWS
