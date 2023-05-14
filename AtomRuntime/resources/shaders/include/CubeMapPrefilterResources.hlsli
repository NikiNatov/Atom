#ifndef __CUBEMAPPREFILTERRESOURCES_HLSLI__
#define __CUBEMAPPREFILTERRESOURCES_HLSLI__

#include "include/Common.hlsli"

struct CubeMapPrefilterResources
{
    float                    Roughness;
    TextureCube              EnvMapUnfiltered;
    RWTexture2DArray<float4> EnvMap;
    SamplerState             EnvMapSampler;
};

// -------------------------------------------------- Constants --------------------------------------------------- //
cbuffer CubeMapPrefilterConstants : register(b0, ATOM_INSTANCE_DATA_SPACE)
{
    float CubeMapPrefilterResources_Roughness;
}

// -------------------------------------------------- Resources --------------------------------------------------- //
TextureCube              CubeMapPrefilterResources_EnvMapUnfiltered : register(t0, ATOM_INSTANCE_DATA_SPACE);
RWTexture2DArray<float4> CubeMapPrefilterResources_EnvMap           : register(u0, ATOM_INSTANCE_DATA_SPACE);

// --------------------------------------------------- Samplers --------------------------------------------------- //
SamplerState CubeMapPrefilterResources_EnvMapSampler : register(s0, ATOM_INSTANCE_DATA_SPACE);


CubeMapPrefilterResources CreateCubeMapPrefilterResources()
{
    CubeMapPrefilterResources resources;
    resources.Roughness = CubeMapPrefilterResources_Roughness;
    resources.EnvMapUnfiltered = CubeMapPrefilterResources_EnvMapUnfiltered;
    resources.EnvMap = CubeMapPrefilterResources_EnvMap;
    resources.EnvMapSampler = CubeMapPrefilterResources_EnvMapSampler;
    return resources;
}

#endif // __CUBEMAPPREFILTERRESOURCES_HLSLI__