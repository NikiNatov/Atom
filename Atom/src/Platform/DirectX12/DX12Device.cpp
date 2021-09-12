#include "atompch.h"

#if defined(ATOM_PLATFORM_WINDOWS)

#include "DX12Device.h"
#include "DX12Adapter.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    DX12Device::DX12Device(const Adapter* adapter)
        : m_Adapter(adapter)
    {
        auto dxgiAdapter = m_Adapter->As<DX12Adapter>()->GetDXGIAdapter();

#if defined(ATOM_DEBUG)

        // Enable debug layer
        wrl::ComPtr<ID3D12Debug> debugInterface;
        DXCall(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));
        debugInterface->EnableDebugLayer();

#endif // ATOM_DEBUG

        D3D_FEATURE_LEVEL featureLevels[] =
        {
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_12_0,
            D3D_FEATURE_LEVEL_12_1
        };

        D3D12_FEATURE_DATA_FEATURE_LEVELS featureLevelInfo = {};
        featureLevelInfo.NumFeatureLevels = _countof(featureLevels);
        featureLevelInfo.pFeatureLevelsRequested = featureLevels;

        // Create a device with the minimum supported feature level
        wrl::ComPtr<ID3D12Device> device = nullptr;
        DXCall(D3D12CreateDevice(dxgiAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)));

        // Get the max supported feature level
        DXCall(device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &featureLevelInfo, sizeof(featureLevelInfo)));

        // Create the main device with the max supported feature level
        ATOM_ENGINE_ASSERT(featureLevelInfo.MaxSupportedFeatureLevel >= D3D_FEATURE_LEVEL_11_0);
        DXCall(D3D12CreateDevice(dxgiAdapter.Get(), featureLevelInfo.MaxSupportedFeatureLevel, IID_PPV_ARGS(&m_D3DDevice)));

        // Get feature options
        DXCall(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &m_FeatureOptions, sizeof(m_FeatureOptions)));

#if defined(ATOM_DEBUG)

        // Set up the message severity levels
        wrl::ComPtr<ID3D12InfoQueue> infoQueue = nullptr;
        DXCall(m_D3DDevice.As(&infoQueue));

        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

#endif // ATOM_DEBUG
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    DX12Device::~DX12Device()
    {
#if defined(ATOM_DEBUG)

        // Reset the message severity levels
        wrl::ComPtr<ID3D12InfoQueue> infoQueue = nullptr;
        DXCall(m_D3DDevice.As(&infoQueue));

        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, false);
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, false);
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, false);

#endif // ATOM_DEBUG
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const Adapter* DX12Device::GetAdapter() const
    {
        return m_Adapter;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool DX12Device::IsRayTracingSupported() const
    {
        return m_FeatureOptions.RaytracingTier > D3D12_RAYTRACING_TIER_1_0;
    }
}

#endif // ATOM_PLATFORM_WINDOWS
