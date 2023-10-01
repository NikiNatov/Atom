#pragma once

#include "Atom/Core/Core.h"

#include "DirectX12.h"

#include "Atom/Renderer/Device.h"
#include "Atom/Renderer/CommandQueue.h"
#include "Atom/Renderer/Texture.h"
#include "Atom/Renderer/CommandBuffer.h"
#include "Atom/Renderer/Pipeline.h"
#include "Atom/Renderer/Buffer.h"
#include "Atom/Renderer/DescriptorHeap.h"
#include "Atom/Renderer/TextureSampler.h"
#include "Atom/Renderer/ResourceState.h"

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
    static D3D12_RESOURCE_DIMENSION AtomTextureTypeToD3D12(TextureType type)
    {
        switch (type)
        {
            case TextureType::Texture2D: return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
            case TextureType::Texture3D: return D3D12_RESOURCE_DIMENSION_TEXTURE3D;
        }

        ATOM_ASSERT(false, "Unknown texture type!");
        return D3D12_RESOURCE_DIMENSION_UNKNOWN;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    static TextureType D3D12TextureTypeAtom(D3D12_RESOURCE_DIMENSION type)
    {
        switch (type)
        {
            case D3D12_RESOURCE_DIMENSION_TEXTURE2D: return TextureType::Texture2D;
            case D3D12_RESOURCE_DIMENSION_TEXTURE3D: return TextureType::Texture3D;
        }

        ATOM_ASSERT(false, "Unknown texture type!");
        return TextureType::None;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    static DXGI_FORMAT AtomTextureFormatToD3D12(TextureFormat format)
    {
        switch (format)
        {
            case TextureFormat::R8:              return DXGI_FORMAT_R8_UNORM;
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
            case DXGI_FORMAT_R8_UNORM:           return TextureFormat::R8;
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
            case TextureFormat::R8:              return DXGI_FORMAT_R8_UNORM;
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
            case TextureFormat::R8:              return DXGI_FORMAT_R8_UNORM;
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
            case TextureFormat::R8:              return DXGI_FORMAT_R8_UNORM;
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
            case TextureFormat::R8:              return DXGI_FORMAT_R8_UNORM;
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

    // -----------------------------------------------------------------------------------------------------------------------------
    static ShaderDataType D3D12ShaderTypeToAtom(D3D12_SHADER_TYPE_DESC typeDesc)
    {
        switch (typeDesc.Type)
        {
            case D3D_SVT_BOOL: 
            {
                return ShaderDataType::Bool;
            }
            case D3D_SVT_INT: 
            {
                if (typeDesc.Columns == 1)
                    return ShaderDataType::Int;
                else if (typeDesc.Columns == 2)
                    return ShaderDataType::Int2;
                else if (typeDesc.Columns == 3)
                    return ShaderDataType::Int3;
                else if (typeDesc.Columns == 4)
                    return ShaderDataType::Int4;

                break;
            }
            case D3D_SVT_FLOAT:
            {
                if (typeDesc.Columns == 1)
                    return ShaderDataType::Float;
                else if (typeDesc.Columns == 2 && typeDesc.Rows == 2)
                    return ShaderDataType::Mat2;
                else if(typeDesc.Columns == 2)
                    return ShaderDataType::Float2;
                else if (typeDesc.Columns == 3 && typeDesc.Rows == 3)
                    return ShaderDataType::Mat3;
                else if (typeDesc.Columns == 3)
                    return ShaderDataType::Float3;
                else if (typeDesc.Columns == 4 && typeDesc.Rows == 4)
                    return ShaderDataType::Mat4;
                else if (typeDesc.Columns == 4)
                    return ShaderDataType::Float4;

                break;
            }
            case D3D_SVT_UINT:
            {
                if (typeDesc.Columns == 1)
                    return ShaderDataType::Uint;
                else if (typeDesc.Columns == 2)
                    return ShaderDataType::Uint2;
                else if (typeDesc.Columns == 3)
                    return ShaderDataType::Uint3;
                else if (typeDesc.Columns == 4)
                    return ShaderDataType::Uint4;

                break;
            }
        }

        return ShaderDataType::None;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    static ShaderResourceType D3D12ShaderResourceTypeToAtom(D3D12_SHADER_INPUT_BIND_DESC typeDesc)
    {
        switch (typeDesc.Type)
        {
            case D3D_SIT_CBUFFER: 
            {
                return ShaderResourceType::ConstantBuffer;
            }
            case D3D_SIT_TEXTURE:
            {
                if (typeDesc.Dimension == D3D_SRV_DIMENSION_TEXTURE2D)
                    return ShaderResourceType::Texture2D;
                else if (typeDesc.Dimension == D3D_SRV_DIMENSION_TEXTURE2DARRAY)
                    return ShaderResourceType::Texture2DArray;
                else if (typeDesc.Dimension == D3D_SRV_DIMENSION_TEXTURECUBE)
                    return ShaderResourceType::TextureCube;

                break;
            }
            case D3D_SIT_UAV_RWTYPED:
            {
                if (typeDesc.Dimension == D3D_SRV_DIMENSION_TEXTURE2D)
                    return ShaderResourceType::RWTexture2D;
                else if (typeDesc.Dimension == D3D_SRV_DIMENSION_TEXTURE2DARRAY)
                    return ShaderResourceType::RWTexture2DArray;

                break;
            }
            case D3D_SIT_SAMPLER:
            {
                return ShaderResourceType::Sampler;
            }
            case D3D_SIT_STRUCTURED:
            {
                return ShaderResourceType::StructuredBuffer;
            }
            case D3D_SIT_UAV_RWSTRUCTURED:
            case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
            {
                return ShaderResourceType::RWStructuredBuffer;
            }
        }

        ATOM_ASSERT(false, "Unknown type!");
        return ShaderResourceType::None;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    static u32 GetTextureFormatSize(TextureFormat format)
    {
        switch (format)
        {
            case TextureFormat::R8:              return 1;
            case TextureFormat::Depth24Stencil8: return 4;
            case TextureFormat::Depth32:         return 4;
            case TextureFormat::RGBA8:           return 4;
            case TextureFormat::RG16F:           return 4;
            case TextureFormat::RG32F:           return 8;
            case TextureFormat::RGBA16F:         return 8;
            case TextureFormat::RGBA32F:         return 16;
        }

        ATOM_ASSERT(false, "Unknown format!");
        return 0;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    static D3D12_RESOURCE_STATES AtomResourceStateToD3D12(ResourceState state)
    {
        D3D12_RESOURCE_STATES d3dState = D3D12_RESOURCE_STATE_COMMON;

        if (IsSet(state, ResourceState::VertexConstantBuffer))
            d3dState |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
        if (IsSet(state, ResourceState::IndexBuffer))
            d3dState |= D3D12_RESOURCE_STATE_INDEX_BUFFER;
        if (IsSet(state, ResourceState::RenderTarget))
            d3dState |= D3D12_RESOURCE_STATE_RENDER_TARGET;
        if (IsSet(state, ResourceState::UnorderedAccess))
            d3dState |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
        if (IsSet(state, ResourceState::DepthWrite))
            d3dState |= D3D12_RESOURCE_STATE_DEPTH_WRITE;
        if (IsSet(state, ResourceState::DepthRead))
            d3dState |= D3D12_RESOURCE_STATE_DEPTH_READ;
        if (IsSet(state, ResourceState::NonPixelShaderRead))
            d3dState |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
        if (IsSet(state, ResourceState::PixelShaderRead))
            d3dState |= D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        if (IsSet(state, ResourceState::StreamOut))
            d3dState |= D3D12_RESOURCE_STATE_STREAM_OUT;
        if (IsSet(state, ResourceState::IndirectArgument))
            d3dState |= D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
        if (IsSet(state, ResourceState::CopyDestination))
            d3dState |= D3D12_RESOURCE_STATE_COPY_DEST;
        if (IsSet(state, ResourceState::CopySource))
            d3dState |= D3D12_RESOURCE_STATE_COPY_SOURCE;
        if (IsSet(state, ResourceState::ResolveDestination))
            d3dState |= D3D12_RESOURCE_STATE_RESOLVE_DEST;
        if (IsSet(state, ResourceState::ResolveSource))
            d3dState |= D3D12_RESOURCE_STATE_RESOLVE_SOURCE;
        if (IsSet(state, ResourceState::RaytracingAccelerationStructure))
            d3dState |= D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
        if (IsSet(state, ResourceState::Present))
            d3dState |= D3D12_RESOURCE_STATE_PRESENT;
        if (IsSet(state, ResourceState::GenericRead))
            d3dState |= D3D12_RESOURCE_STATE_GENERIC_READ;
        if (IsSet(state, ResourceState::AnyShaderRead))
            d3dState |= D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;
        if (IsSet(state, ResourceState::Predication))
            d3dState |= D3D12_RESOURCE_STATE_PREDICATION;

        return d3dState;
    }
}}
