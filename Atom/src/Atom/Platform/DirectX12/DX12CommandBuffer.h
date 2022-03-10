#pragma once

#include "Atom/Renderer/API/CommandBuffer.h"

#if defined(ATOM_PLATFORM_WINDOWS)

#include "DirectX12.h"

namespace Atom
{
    class DX12CommandBuffer : public CommandBuffer
    {
    public:
        DX12CommandBuffer();
        ~DX12CommandBuffer();

        virtual void Begin() override;
        virtual void TransitionResource(const Ref<Texture2D>& texture, ResourceState beforeState, ResourceState afterState) override;
        virtual void End() override;

        inline wrl::ComPtr<ID3D12GraphicsCommandList6> GetCommandList() const { return m_CommandList; }
    private:
        Vector<wrl::ComPtr<ID3D12CommandAllocator>> m_Allocators;
        wrl::ComPtr<ID3D12GraphicsCommandList6>     m_CommandList;
        Vector<u64>                                 m_FenceValues;
    };
}

#endif // ATOM_PLATFORM_WINDOWS