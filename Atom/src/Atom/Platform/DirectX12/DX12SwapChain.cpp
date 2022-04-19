#include "atompch.h"

#if defined(ATOM_PLATFORM_WINDOWS)

#include "DX12SwapChain.h"
#include "DX12Device.h"
#include "DX12CommandQueue.h"
#include "DX12TextureView.h"
#include "DX12ResourceStateTracker.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    DX12SwapChain::DX12SwapChain(u64 windowHandle, u32 width, u32 height)
        : m_Width(width), m_Height(height)
    {
        auto dxgiFactory = Renderer::GetDevice().As<DX12Device>()->GetDXGIFactory();
        auto gfxQueue = Renderer::GetDevice().GetCommandQueue(CommandQueueType::Graphics).As<DX12CommandQueue>()->GetD3DCommandQueue();

        // Check for tearing support
        ComPtr<IDXGIFactory5> factory;
        dxgiFactory.As(&factory);
        DXCall(factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &m_TearingSupported, sizeof(u32)));

        // Create the swap chain
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.BufferCount = Renderer::GetFramesInFlight();
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
        u32 framesInFlight = Renderer::GetFramesInFlight();
        m_BackBuffers.resize(framesInFlight, nullptr);
        m_BackBufferRTVs.resize(framesInFlight, nullptr);
        RecreateBuffers();

        // Initialize fence values
        m_FrameFenceValues.resize(framesInFlight, 0);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    DX12SwapChain::~DX12SwapChain()
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void DX12SwapChain::Present(bool vsync)
    {
        auto& gfxQueue = Renderer::GetDevice().GetCommandQueue(CommandQueueType::Graphics);

        // Present and signal the fence for the current frame
        DXCall(m_DXGISwapChain->Present(vsync, !vsync && m_TearingSupported ? DXGI_PRESENT_ALLOW_TEARING : 0));
        m_FrameFenceValues[m_BackBufferIndex] = gfxQueue.Signal();

        // Update the back buffer index
        m_BackBufferIndex = m_DXGISwapChain->GetCurrentBackBufferIndex();

        // If the next frame is not finished rendering, wait
        gfxQueue.WaitForFenceValue(m_FrameFenceValues[m_BackBufferIndex]);

        // Process deferred releases for descriptors
        Renderer::GetDevice().ProcessDeferredReleases(m_BackBufferIndex);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void DX12SwapChain::Resize(u32 width, u32 height)
    {
        if (m_Width != width || m_Height != height)
        {
            m_Width = width;
            m_Height = height;

            // Wait for GPU to finish all work
            Renderer::GetDevice().WaitIdle();

            for (u32 i = 0 ; i < m_BackBuffers.size(); i++)
            {
                DX12ResourceStateTracker::RemoveGlobalResourceState(m_BackBuffers[i]->As<DX12Texture>()->GetD3DResource().Get());

                m_BackBuffers[i].reset();
                m_BackBufferRTVs[i]->As<DX12TextureViewRT>()->GetDescriptor().Release();
                m_BackBufferRTVs[i].reset();
            }

            u32 flags = m_TearingSupported ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
            DXCall(m_DXGISwapChain->ResizeBuffers(m_BackBuffers.size(), 0, 0, DXGI_FORMAT_UNKNOWN, flags));
            m_BackBufferIndex = m_DXGISwapChain->GetCurrentBackBufferIndex();

            RecreateBuffers();
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 DX12SwapChain::GetWidth() const
    {
        return m_Width;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 DX12SwapChain::GetHeight() const
    {
        return m_Height;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 DX12SwapChain::GetCurrentBackBufferIndex() const
    {
        return m_BackBufferIndex;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 DX12SwapChain::GetBackBufferCount() const
    {
        return m_BackBuffers.size();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const Ref<Texture>& DX12SwapChain::GetBackBuffer() const
    {
        return m_BackBuffers[m_BackBufferIndex];
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const Ref<TextureViewRT>& DX12SwapChain::GetBackBufferRTV() const
    {
        return m_BackBufferRTVs[m_BackBufferIndex];
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void DX12SwapChain::RecreateBuffers()
    {
        // Recreate resources
        for (u32 i = 0 ; i < m_BackBuffers.size(); i++)
        {
            // Get the back buffer resource
            ComPtr<ID3D12Resource2> backBuffer = nullptr;
            DXCall(m_DXGISwapChain->GetBuffer(i, IID_PPV_ARGS(backBuffer.GetAddressOf())));
            m_BackBuffers[i] = Texture::CreateSwapChainBuffer((u64)backBuffer.Detach());
            m_BackBufferRTVs[i] = TextureViewRT::Create(m_BackBuffers[i]);

            DX12ResourceStateTracker::AddGlobalResourceState(m_BackBuffers[i]->As<DX12Texture>()->GetD3DResource().Get(), D3D12_RESOURCE_STATE_PRESENT);
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

#endif // ATOM_PLATFORM_WINDOWS