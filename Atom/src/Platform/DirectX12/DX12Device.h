#pragma once

#include "Atom/Renderer/Device.h"

#if defined(ATOM_PLATFORM_WINDOWS)

#include "DirectX12.h"

namespace Atom
{
    class DX12Device : public Device
    {
    public:
        DX12Device(const Adapter* adapter);
        ~DX12Device();

        virtual const Adapter* GetAdapter() const override;
        virtual bool IsRayTracingSupported() const override;

        inline wrl::ComPtr<ID3D12Device6> GetD3DDevice() const { return m_D3DDevice; }
    private:
        const Adapter*                    m_Adapter;
        wrl::ComPtr<ID3D12Device6>        m_D3DDevice;
        D3D12_FEATURE_DATA_D3D12_OPTIONS5 m_FeatureOptions;
    };

}

#endif // ATOM_PLATFORM_WINDOWS