#include "atompch.h"

#if defined(ATOM_PLATFORM_WINDOWS)

#include "DX12Adapter.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    DX12Adapter::DX12Adapter(AdapterPreference preference)
    {
        wrl::ComPtr<IDXGIFactory6> dxgiFactory = nullptr;
        wrl::ComPtr<IDXGIAdapter> dxgiAdapter = nullptr;

        u32 factoryCreateFlags = 0;

#if defined(ATOM_DEBUG)
        factoryCreateFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif // ATOM_DEBUG

        // Create factory
        DXCall(CreateDXGIFactory2(factoryCreateFlags, IID_PPV_ARGS(&dxgiFactory)));

        u64 currentMaxVideoMemory = 0;

        // Query all adapters and pick the one with the most video memory
        for (u32 i = 0; dxgiFactory->EnumAdapterByGpuPreference(i, Utils::AtomAdapterPreferenceToDXGI(preference), IID_PPV_ARGS(&dxgiAdapter)) != DXGI_ERROR_NOT_FOUND; i++)
        {
            DXGI_ADAPTER_DESC desc;
            dxgiAdapter->GetDesc(&desc);
          
            if (SUCCEEDED(D3D12CreateDevice(dxgiAdapter.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)) && desc.DedicatedVideoMemory > currentMaxVideoMemory)
            {
                currentMaxVideoMemory = desc.DedicatedVideoMemory;
                DXCall(dxgiAdapter.As(&m_DXGIAdapter));
            }
        }

        // Get the adapter description
        m_DXGIAdapter->GetDesc3(&m_Description);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    DX12Adapter::~DX12Adapter()
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u64 DX12Adapter::GetSystemMemory() const
    {
        return m_Description.DedicatedSystemMemory;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u64 DX12Adapter::GetVideoMemory() const
    {
        return m_Description.DedicatedVideoMemory;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u64 DX12Adapter::GetSharedMemory() const
    {
        return m_Description.SharedSystemMemory;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    String DX12Adapter::GetDescription() const
    {
        std::wstring desc = m_Description.Description;
        return std::string(desc.begin(), desc.end());;
    }
}

#endif // ATOM_PLATFORM_WINDOWS
