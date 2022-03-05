#include "atompch.h"
#include "Renderer.h"

#include "Atom/Core/Application.h"
#include "Atom/Renderer/API/Device.h"
#include "Atom/Renderer/API/CommandBuffer.h"

#include "Atom/Platform/DirectX12/DX12CommandBuffer.h"
#include "Atom/Platform/DirectX12/DX12SwapChain.h"

namespace Atom
{
    RenderAPI          Renderer::ms_RenderAPI = RenderAPI::None;
    Ref<CommandBuffer> Renderer::ms_CommandBuffer = nullptr;

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::Initialize()
    {
        ms_CommandBuffer = CommandBuffer::Create();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::BeginFrame()
    {
        auto swapChain = Application::Get().GetWindow().GetSwapChain().As<DX12SwapChain>();
        ms_CommandBuffer->Begin();

        auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(swapChain->GetBackBuffer(Renderer::GetCurrentFrameIndex()).Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        ms_CommandBuffer->As<DX12CommandBuffer>()->GetCommandList()->ResourceBarrier(1, &barrier);

        const f32 color[] = { 1.0f, 0.0f, 0.0f, 1.0f };
        ms_CommandBuffer->As<DX12CommandBuffer>()->GetCommandList()->ClearRenderTargetView(swapChain->GetBackBufferRTV(Renderer::GetCurrentFrameIndex()).GetCPUHandle(), color, 1, &swapChain->GetScissorRect());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::EndFrame()
    {
        auto swapChain = Application::Get().GetWindow().GetSwapChain().As<DX12SwapChain>();

        auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(swapChain->GetBackBuffer(Renderer::GetCurrentFrameIndex()).Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        ms_CommandBuffer->As<DX12CommandBuffer>()->GetCommandList()->ResourceBarrier(1, &barrier);

        ms_CommandBuffer->End();

        Application::Get().GetWindow().GetDevice().GetCommandQueue(CommandQueueType::Graphics).ExecuteCommandList(ms_CommandBuffer);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Device& Renderer::GetDevice()
    {
        return Application::Get().GetWindow().GetDevice();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 Renderer::GetCurrentFrameIndex()
    {
        return Application::Get().GetWindow().GetSwapChain().GetCurrentBackBufferIndex();
    }
}