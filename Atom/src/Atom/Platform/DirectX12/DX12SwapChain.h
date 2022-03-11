#pragma once

#include "Atom/Renderer/API/SwapChain.h"

#if defined(ATOM_PLATFORM_WINDOWS)

#include "DirectX12.h"
#include "DX12DescriptorAllocator.h"
#include "DX12Texture.h"

namespace Atom
{
    class DX12SwapChain : public SwapChain
    {
    public:
        DX12SwapChain(Device& device, u64 windowHandle, u32 width, u32 height);
        ~DX12SwapChain();

        virtual void Present(bool vSync = true) override;
        virtual void Resize(u32 width, u32 height) override;
        virtual u32 GetWidth() const override;
        virtual u32 GetHeight() const override;
        virtual u32 GetCurrentBackBufferIndex() const override;
        virtual u32 GetBackBufferCount() const override;

        inline IDXGISwapChain4* GetDXGISwapChain() const { return m_DXGISwapChain; }
        inline const D3D12_VIEWPORT& GetViewport() const { return m_Viewport; }
        inline const D3D12_RECT& GetScissorRect() const { return m_ScissorRect; }

        inline const Ref<Texture2D>& GetBackBuffer(u32 bufferIndex) const 
        { 
            ATOM_ASSERT(bufferIndex >= 0 && bufferIndex < m_BackBuffers.size()); 
            return m_BackBuffers[bufferIndex]; 
        }
    private:
        void RecreateBuffers();
    private:
        IDXGISwapChain4*           m_DXGISwapChain;
        u32                        m_Width;
        u32                        m_Height;
        D3D12_VIEWPORT             m_Viewport;
        D3D12_RECT                 m_ScissorRect;
        u32                        m_TearingSupported;
        u32                        m_BackBufferIndex;
        Vector<Ref<DX12Texture2D>> m_BackBuffers;
        Vector<u64>                m_FrameFenceValues;
        DX12Device&                m_Device;
    };
}

#endif // ATOM_PLATFORM_WINDOWS