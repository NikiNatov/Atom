#ifndef __CUBEMAPIRRADIANCERESOURCES_HLSLI__
#define __CUBEMAPPREFILTERRESOURCES_HLSLI__

#include "include/Common.hlsli"

struct CubeMapIrradianceResources
{
    TextureCube              EnvMap;
    RWTexture2DArray<float4> IrradianceMap;
    SamplerState             EnvMapSampler;
};

// -------------------------------------------------- Resources --------------------------------------------------- //
TextureCube              CubeMapIrradianceResources_EnvMap        : register(t0, ATOM_INSTANCE_DATA_SPACE);
RWTexture2DArray<float4> CubeMapIrradianceResources_IrradianceMap : register(u0, ATOM_INSTANCE_DATA_SPACE);

// --------------------------------------------------- Samplers --------------------------------------------------- //
SamplerState CubeMapIrradianceResources_EnvMapSampler : register(s0, ATOM_INSTANCE_DATA_SPACE);

CubeMapIrradianceResources CreateCubeMapIrradianceResources()
{
    CubeMapIrradianceResources resources;
    resources.EnvMap = CubeMapIrradianceResources_EnvMap;
    resources.IrradianceMap = CubeMapIrradianceResources_IrradianceMap;
    resources.EnvMapSampler = CubeMapIrradianceResources_EnvMapSampler;
    return resources;
}

#endif // __CUBEMAPPREFILTERRESOURCES_HLSLI__