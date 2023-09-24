#include "atompch.h"
#include "Fence.h"

#include "Atom/Renderer/Device.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    Fence::Fence(const char* debugName)
        : m_TargetValue(0)
    {
        auto d3dDevice = Device::Get().GetD3DDevice();
        DXCall(d3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_D3DFence)));

#if defined (ATOM_DEBUG)
        String name = debugName;
        DXCall(m_D3DFence->SetName(STRING_TO_WSTRING(name).c_str()));
#endif
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Fence::WaitForValueCPU(u64 value)
    {
        if (m_D3DFence->GetCompletedValue() < value)
        {
            if (HANDLE fenceEvent = CreateEvent(0, false, false, 0))
            {
                DXCall(m_D3DFence->SetEventOnCompletion(value, fenceEvent));
                WaitForSingleObject(fenceEvent, INFINITE);
                CloseHandle(fenceEvent);
            }
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u64 Fence::IncrementTargetValue()
    {
        return ++m_TargetValue;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool Fence::IsTargetValueReached() const
    {
        return m_D3DFence->GetCompletedValue() >= m_TargetValue;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u64 Fence::GetCompletedValue() const
    {
        return m_D3DFence->GetCompletedValue();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u64 Fence::GetTargetValue() const
    {
        return m_TargetValue;
    }
}
