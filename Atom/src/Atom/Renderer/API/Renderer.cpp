#include "atompch.h"
#include "Renderer.h"

#include "Atom/Core/Application.h"
#include "Atom/Renderer/API/Device.h"
#include "Atom/Renderer/API/CommandBuffer.h"
#include "Atom/Renderer/API/Texture.h"

#include "Atom/Platform/DirectX12/DX12CommandBuffer.h"
#include "Atom/Platform/DirectX12/DX12SwapChain.h"
#include "Atom/Platform/DirectX12/DX12Texture.h"

namespace Atom
{
    RenderAPI Renderer::ms_RenderAPI = RenderAPI::None;

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::Initialize()
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::BeginFrame(const Ref<CommandBuffer>& commandBuffer)
    {
        auto swapChain = Application::Get().GetWindow().GetSwapChain().As<DX12SwapChain>();
        commandBuffer->Begin();

        auto backBuffer = swapChain->GetBackBuffer(Renderer::GetCurrentFrameIndex())->As<DX12Texture2D>();
        commandBuffer->TransitionResource(swapChain->GetBackBuffer(Renderer::GetCurrentFrameIndex()), ResourceState::Present, ResourceState::RenderTarget);

        const f32 color[] = { 0.2f, 0.2f, 0.2f, 1.0f };
        commandBuffer->As<DX12CommandBuffer>()->GetCommandList()->ClearRenderTargetView(backBuffer->GetRenderTargetView(0).GetCPUHandle(), color, 1, &swapChain->GetScissorRect());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::EndFrame(const Ref<CommandBuffer>& commandBuffer)
    {
        auto swapChain = Application::Get().GetWindow().GetSwapChain().As<DX12SwapChain>();

        commandBuffer->TransitionResource(swapChain->GetBackBuffer(Renderer::GetCurrentFrameIndex()), ResourceState::RenderTarget, ResourceState::Present);

        commandBuffer->End();

        Application::Get().GetWindow().GetDevice().GetCommandQueue(CommandQueueType::Graphics).ExecuteCommandList(commandBuffer);
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