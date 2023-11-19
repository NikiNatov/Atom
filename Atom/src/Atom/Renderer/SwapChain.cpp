#include "atompch.h"

#include "SwapChain.h"
#include "Device.h"
#include "CommandQueue.h"
#include "Texture.h"
#include "ResourceStateTracker.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    SwapChain::SwapChain(HWND windowHandle, u32 width, u32 height)
        : m_Width(width), m_Height(height)
    {
        auto dxgiFactory = Device::Get().GetDXGIFactory();
        auto gfxQueue = Device::Get().GetCommandQueue(CommandQueueType::Graphics)->GetD3DCommandQueue();

        // Check for tearing support
        ComPtr<IDXGIFactory5> factory;
        dxgiFactory.As(&factory);
        DXCall(factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &m_TearingSupported, sizeof(u32)));

        // Create the swap chain
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.BufferCount = g_FramesInFlight;
        swapChainDesc.Width = m_Width;
        swapChainDesc.Height = m_Height;
        swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.Flags = m_TearingSupported ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
        swapChainDesc.Scaling = DXGI_SCALING_STRETCH;

        ComPtr<IDXGISwapChain1> swapChain;
        DXCall(dxgiFactory->CreateSwapChainForHwnd(gfxQueue.Get(), (HWND)windowHandle, &swapChainDesc, nullptr, nullptr, &swapChain));

        // Disable exclusive fullscreen mode
        DXCall(dxgiFactory->MakeWindowAssociation((HWND)windowHandle, DXGI_MWA_NO_ALT_ENTER));

        DXCall(swapChain.As(&m_DXGISwapChain));
        m_BackBufferIndex = m_DXGISwapChain->GetCurrentBackBufferIndex();

        // Create the back buffers
        RecreateBuffers();

        // Create frame fence
        m_FrameFence = CreateRef<Fence>("Frame fence");

        // Initialize fence values
        memset(m_FrameFenceValues, 0, g_FramesInFlight * sizeof(u64));
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    SwapChain::~SwapChain()
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SwapChain::Present(bool vsync)
    {
        auto gfxQueue = Device::Get().GetCommandQueue(CommandQueueType::Graphics);

        // Present and signal the fence for the current frame
        DXCall(m_DXGISwapChain->Present(vsync, !vsync && m_TearingSupported ? DXGI_PRESENT_ALLOW_TEARING : 0));
        m_FrameFenceValues[m_BackBufferIndex] = m_FrameFence->IncrementTargetValue();
        gfxQueue->SignalFence(m_FrameFence, m_FrameFenceValues[m_BackBufferIndex]);

        // Update the back buffer index
        m_BackBufferIndex = m_DXGISwapChain->GetCurrentBackBufferIndex();

        // If the next frame is not finished rendering, wait
        m_FrameFence->WaitForValueCPU(m_FrameFenceValues[m_BackBufferIndex]);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SwapChain::Resize(u32 width, u32 height)
    {
        if (m_Width != width || m_Height != height)
        {
            m_Width = width;
            m_Height = height;

            // Wait for GPU to finish all work
            Device::Get().WaitIdle();

            for (u32 i = 0; i < g_FramesInFlight; i++)
            {
                ResourceStateTracker::RemoveGlobalResourceState(m_BackBuffers[i]->GetTexture().get());
                m_BackBuffers[i].reset();
            }

            u32 flags = m_TearingSupported ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
            DXCall(m_DXGISwapChain->ResizeBuffers(g_FramesInFlight, 0, 0, DXGI_FORMAT_UNKNOWN, flags));
            m_BackBufferIndex = m_DXGISwapChain->GetCurrentBackBufferIndex();

            RecreateBuffers();
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 SwapChain::GetWidth() const
    {
        return m_Width;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 SwapChain::GetHeight() const
    {
        return m_Height;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 SwapChain::GetCurrentBackBufferIndex() const
    {
        return m_BackBufferIndex;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<RenderSurface> SwapChain::GetBackBuffer() const
    {
        return m_BackBuffers[m_BackBufferIndex];
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SwapChain::RecreateBuffers()
    {
        // Recreate resources
        for (u32 i = 0; i < g_FramesInFlight; i++)
        {
            // Get the back buffer resource
            ComPtr<ID3D12Resource2> backBuffer = nullptr;
            DXCall(m_DXGISwapChain->GetBuffer(i, IID_PPV_ARGS(backBuffer.GetAddressOf())));
            m_BackBuffers[i] = CreateRef<RenderSurface>(backBuffer.Detach(), true, fmt::format("SwapChainBackBuffer[{}]", i).c_str());
        }

        // Recreate viewport
        m_Viewport.TopLeftX = 0;
        m_Viewport.TopLeftY = 0;
        m_Viewport.Width = m_Width;
        m_Viewport.Height = m_Height;
        m_Viewport.MinDepth = 0.0f;
        m_Viewport.MaxDepth = 1.0f;

        // Recreate scissor rect
        m_ScissorRect = { 0, 0, (s32)m_Width, (s32)m_Height };
    }
}
