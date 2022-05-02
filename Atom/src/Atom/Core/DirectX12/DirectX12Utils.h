#pragma once

#include "Atom/Core/Core.h"

#include "DirectX12.h"

#include "Atom/Renderer/Device.h"
#include "Atom/Renderer/CommandQueue.h"
#include "Atom/Renderer/Texture.h"
#include "Atom/Renderer/CommandBuffer.h"
#include "Atom/Renderer/GraphicsPipeline.h"
#include "Atom/Renderer/Buffer.h"
#include "Atom/Renderer/DescriptorHeap.h"

namespace Atom { namespace Utils {

    // -----------------------------------------------------------------------------------------------------------------------------
    static DXGI_GPU_PREFERENCE AtomGPUPreferenceToDXGI(GPUPreference preference)
    {
        switch (preference)
        {
            case GPUPreference::None:            return DXGI_GPU_PREFERENCE_UNSPECIFIED;
            case GPUPreference::MinimumPowered:  return DXGI_GPU_PREFERENCE_MINIMUM_POWER;
            case GPUPreference::HighPerformance: return DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE;
        }

        ATOM_ASSERT(false, "Unknown GPU preference!");
        return DXGI_GPU_PREFERENCE_UNSPECIFIED;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    static D3D12_COMMAND_LIST_TYPE AtomCommandQueueTypeToD3D12(CommandQueueType type)
    {
        switch (type)
        {
            case CommandQueueType::Graphics: return D3D12_COMMAND_LIST_TYPE_DIRECT;
            case CommandQueueType::Compute:  return D3D12_COMMAND_LIST_TYPE_COMPUTE;
            case CommandQueueType::Copy:     return D3D12_COMMAND_LIST_TYPE_COPY;
        }

        ATOM_ASSERT(false, "Unknown queue type!");
        return D3D12_COMMAND_LIST_TYPE_DIRECT;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    static DXGI_FORMAT AtomTextureFormatToD3D12(TextureFormat format)
    {
        switch (format)
        {
            case TextureFormat::RGBA8:           return DXGI_FORMAT_R8G8B8A8_UNORM;
            case TextureFormat::RG16F:           return DXGI_FORMAT_R16G16_FLOAT;
            case TextureFormat::RGBA16F:         return DXGI_FORMAT_R16G16B16A16_FLOAT;
            case TextureFormat::RG32F:           return DXGI_FORMAT_R32G32_FLOAT;
            case TextureFormat::RGBA32F:         return DXGI_FORMAT_R32G32B32A32_FLOAT;
            case TextureFormat::Depth24Stencil8: return DXGI_FORMAT_R24G8_TYPELESS;
            case TextureFormat::Depth32:         return DXGI_FORMAT_R32_TYPELESS;
        }

        ATOM_ASSERT(false, "Unknown texture format!");
        return DXGI_FORMAT_UNKNOWN;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    static TextureFormat DXGITextureFormatToAtomFormat(DXGI_FORMAT format)
    {
        switch (format)
        {
            case DXGI_FORMAT_R8G8B8A8_UNORM:     return TextureFormat::RGBA8;
            case DXGI_FORMAT_R16G16_FLOAT:       return TextureFormat::RG16F;
            case DXGI_FORMAT_R16G16B16A16_FLOAT: return TextureFormat::RGBA16F;
            case DXGI_FORMAT_R32G32_FLOAT:       return TextureFormat::RG32F;
            case DXGI_FORMAT_R32G32B32A32_FLOAT: return TextureFormat::RGBA32F;
            case DXGI_FORMAT_R24G8_TYPELESS:     return TextureFormat::Depth24Stencil8;
            case DXGI_FORMAT_R32_TYPELESS:       return TextureFormat::Depth32;
        }

        ATOM_ASSERT(false, "Unknown texture format!");
        return TextureFormat::RGBA8;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    static DXGI_FORMAT AtomTextureFormatToSRVFormat(TextureFormat format)
    {
        switch (format)
        {
            case TextureFormat::RGBA8:           return DXGI_FORMAT_R8G8B8A8_UNORM;
            case TextureFormat::RG16F:           return DXGI_FORMAT_R16G16_FLOAT;
            case TextureFormat::RGBA16F:         return DXGI_FORMAT_R16G16B16A16_FLOAT;
            case TextureFormat::RG32F:           return DXGI_FORMAT_R32G32_FLOAT;
            case TextureFormat::RGBA32F:         return DXGI_FORMAT_R32G32B32A32_FLOAT;
            case TextureFormat::Depth24Stencil8: return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
            case TextureFormat::Depth32:         return DXGI_FORMAT_R32_FLOAT;
        }

        ATOM_ASSERT(false, "Unknown texture format!");
        return DXGI_FORMAT_UNKNOWN;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    static DXGI_FORMAT AtomTextureFormatToRTVFormat(TextureFormat format)
    {
        switch (format)
        {
            case TextureFormat::RGBA8:           return DXGI_FORMAT_R8G8B8A8_UNORM;
            case TextureFormat::RG16F:           return DXGI_FORMAT_R16G16_FLOAT;
            case TextureFormat::RGBA16F:         return DXGI_FORMAT_R16G16B16A16_FLOAT;
            case TextureFormat::RG32F:           return DXGI_FORMAT_R32G32_FLOAT;
            case TextureFormat::RGBA32F:         return DXGI_FORMAT_R32G32B32A32_FLOAT;
            case TextureFormat::Depth24Stencil8: return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
            case TextureFormat::Depth32:         return DXGI_FORMAT_R32_FLOAT;
        }

        ATOM_ASSERT(false, "Unknown texture format!");
        return DXGI_FORMAT_UNKNOWN;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    static DXGI_FORMAT AtomTextureFormatToDSVFormat(TextureFormat format)
    {
        switch (format)
        {
            case TextureFormat::RGBA8:           return DXGI_FORMAT_R8G8B8A8_UNORM;
            case TextureFormat::RG16F:           return DXGI_FORMAT_R16G16_FLOAT;
            case TextureFormat::RGBA16F:         return DXGI_FORMAT_R16G16B16A16_FLOAT;
            case TextureFormat::RG32F:           return DXGI_FORMAT_R32G32_FLOAT;
            case TextureFormat::RGBA32F:         return DXGI_FORMAT_R32G32B32A32_FLOAT;
            case TextureFormat::Depth24Stencil8: return DXGI_FORMAT_D24_UNORM_S8_UINT;
            case TextureFormat::Depth32:         return DXGI_FORMAT_D32_FLOAT;
        }

        ATOM_ASSERT(false, "Unknown texture format!");
        return DXGI_FORMAT_UNKNOWN;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    static DXGI_FORMAT AtomTextureFormatToUAVFormat(TextureFormat format)
    {
        switch (format)
        {
            case TextureFormat::RGBA8:           return DXGI_FORMAT_R8G8B8A8_UNORM;
            case TextureFormat::RG16F:           return DXGI_FORMAT_R16G16_FLOAT;
            case TextureFormat::RGBA16F:         return DXGI_FORMAT_R16G16B16A16_FLOAT;
            case TextureFormat::RG32F:           return DXGI_FORMAT_R32G32_FLOAT;
            case TextureFormat::RGBA32F:         return DXGI_FORMAT_R32G32B32A32_FLOAT;
            case TextureFormat::Depth24Stencil8: return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
            case TextureFormat::Depth32:         return DXGI_FORMAT_R32_FLOAT;
        }

        ATOM_ASSERT(false, "Unknown texture format!");
        return DXGI_FORMAT_UNKNOWN;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    static D3D12_TEXTURE_ADDRESS_MODE AtomTextureWrapToD3D12(TextureWrap wrapMode)
    {
        switch (wrapMode)
        {
            case TextureWrap::Repeat: return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            case TextureWrap::Clamp:  return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        }

        ATOM_ASSERT(false, "Unknown texture wrap mode!");
        return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    static D3D12_FILTER AtomTextureFilterToD3D12(TextureFilter filter)
    {
        switch (filter)
        {
            case TextureFilter::Linear:      return D3D12_FILTER_MIN_MAG_MIP_LINEAR;
            case TextureFilter::Nearest:     return D3D12_FILTER_MIN_MAG_MIP_POINT;
            case TextureFilter::Anisotropic: return D3D12_FILTER_ANISOTROPIC;
        }

        ATOM_ASSERT(false, "Unknown texture filter!");
        return D3D12_FILTER_MIN_MAG_MIP_POINT;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    static D3D12_PRIMITIVE_TOPOLOGY_TYPE AtomTopologyToD3D12(Topology topology)
    {
        switch (topology)
        {
            case Topology::Triangles: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
            case Topology::Points:    return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
            case Topology::Lines:	  return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
        }

        ATOM_ASSERT(false, "Unknown topology type!");
        return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    static D3D12_PRIMITIVE_TOPOLOGY D3D12TopologyTypeToD3D12Topology(D3D12_PRIMITIVE_TOPOLOGY_TYPE type)
    {
        switch (type)
        {
            case D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT:    return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
            case D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE:     return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
            case D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        }

        ATOM_ASSERT(false, "Unknown topology type!");
        return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    static DXGI_FORMAT AtomIndexBufferFormatToD3D12(IndexBufferFormat format)
    {
        switch (format)
        {
            case IndexBufferFormat::U16: return DXGI_FORMAT_R16_UINT;
            case IndexBufferFormat::U32: return DXGI_FORMAT_R32_UINT;
        }

        ATOM_ASSERT(false, "Unknown index buffer format!");
        return DXGI_FORMAT_UNKNOWN;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    static D3D12_DESCRIPTOR_HEAP_TYPE AtomDescriptorHeapTypeToD3D12(DescriptorHeapType type)
    {
        switch (type)
        {
            case DescriptorHeapType::ShaderResource: return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            case DescriptorHeapType::RenderTarget:   return D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            case DescriptorHeapType::DepthStencil:   return D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
            case DescriptorHeapType::Sampler:        return D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
        }

        ATOM_ASSERT(false, "Unknown heap type!");
        return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    }
}}
