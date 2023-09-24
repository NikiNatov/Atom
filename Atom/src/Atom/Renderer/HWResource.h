#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Core/DirectX12/DirectX12.h"

namespace Atom
{
    class HWResource
    {
    public:
        virtual ~HWResource() = default;

        void SetName(const String& name) { DXCall(m_D3DResource->SetName(STRING_TO_WSTRING(name).c_str())); }

        inline ComPtr<ID3D12Resource> GetD3DResource() const { return m_D3DResource; }
    protected:
        HWResource() = default;
    protected:
        ComPtr<ID3D12Resource> m_D3DResource = nullptr;
    };
}