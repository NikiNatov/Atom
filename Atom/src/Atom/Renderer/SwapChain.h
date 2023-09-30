#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Core/DirectX12/DirectX12.h"
#include "Atom/Renderer/RenderSurface.h"
#include "Atom/Renderer/Fence.h"

namespace Atom
{
    class SwapChain
    {
    public:
        SwapChain(HWND windowHandle, u32 width, u32 height);
        ~SwapChain();

        void Present(bool vSync = true);
        void Resize(u32 width, u32 height);
        u32 GetWidth() const;
        u32 GetHeight() const;
        u32 GetCurrentBackBufferIndex() const;
        u32 GetBackBufferCount() const;
        Ref<RenderSurface> GetBackBuffer() const;
        inline ComPtr<IDXGISwapChain4> GetDXGISwapChain() const { return m_DXGISwapChain; }
        inline const D3D12_VIEWPORT& GetViewport() const { return m_Viewport; }
        inline const D3D12_RECT& GetScissorRect() const { return m_ScissorRect; }
    private:
        void RecreateBuffers();
    private:
        ComPtr<IDXGISwapChain4>    m_DXGISwapChain;
        u32                        m_Width;
        u32                        m_Height;
        D3D12_VIEWPORT             m_Viewport;
        D3D12_RECT                 m_ScissorRect;
        u32                        m_TearingSupported;
        u32                        m_BackBufferIndex;
        Vector<Ref<RenderSurface>> m_BackBuffers;
        Ref<Fence>                 m_FrameFence;
        Vector<u64>                m_FrameFenceValues;
    };
}