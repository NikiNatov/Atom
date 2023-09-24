#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Core/DirectX12/DirectX12.h"

namespace Atom
{
    class Fence
    {
    public:
        Fence(const char* debugName = "Unnamed Fence");
        
        void WaitForValueCPU(u64 value);

        u64 IncrementTargetValue();
        bool IsTargetValueReached() const;
        u64 GetCompletedValue() const;
        u64 GetTargetValue() const;

        inline ComPtr<ID3D12Fence> GetD3DFence() const { return m_D3DFence; }
    private:
        ComPtr<ID3D12Fence> m_D3DFence;
        u64                 m_TargetValue;
    };
}