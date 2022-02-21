#pragma once

#include "Atom/Renderer/API/Adapter.h"

#if defined(ATOM_PLATFORM_WINDOWS)

#include "DirectX12.h"

namespace Atom
{
    class DX12Adapter : public Adapter
    {
    public:
        DX12Adapter(AdapterPreference preference);
        ~DX12Adapter();

        virtual u64 GetSystemMemory() const override;
        virtual u64 GetVideoMemory() const override;
        virtual u64 GetSharedMemory() const override;
        virtual String GetDescription() const override;

        inline wrl::ComPtr<IDXGIAdapter4> GetDXGIAdapter() const { return m_DXGIAdapter; }
    private:
        DXGI_ADAPTER_DESC3         m_Description;
        wrl::ComPtr<IDXGIAdapter4> m_DXGIAdapter = nullptr;
    };

}
#endif // ATOM_PLATFORM_WINDOWS