#include "atompch.h"

#if defined(ATOM_PLATFORM_WINDOWS)

#include "DX12CommandBuffer.h"
#include "DX12Device.h"
#include "DX12Texture.h"

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
        for (auto allocator : m_Allocators)
        {
            COMSafeRelease(allocator);
        }

        COMSafeRelease(m_CommandList);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void DX12CommandBuffer::Begin()
    {
        u32 frameIndex = Renderer::GetCurrentFrameIndex();
        DXCall(m_Allocators[frameIndex]->Reset());
        DXCall(m_CommandList->Reset(m_Allocators[frameIndex], NULL));
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void DX12CommandBuffer::TransitionResource(const Ref<Texture2D>& texture, ResourceState beforeState, ResourceState afterState)
    {
        DX12Texture2D* dx12Texture = texture->As<DX12Texture2D>();

        auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(dx12Texture->GetD3DResource(), 
                                                            Utils::AtomResourceStateToD3D12(beforeState), 
                                                            Utils::AtomResourceStateToD3D12(afterState));
        m_CommandList->ResourceBarrier(1, &barrier);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void DX12CommandBuffer::End()
    {
        DXCall(m_CommandList->Close());
    }
}

#endif // ATOM_PLATFORM_WINDOWS
