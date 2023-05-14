#ifndef __EQUIRECTTOCUBEMAPRESOURCES_HLSLI__
#define __EQUIRECTTOCUBEMAPRESOURCES_HLSLI__

#include "include/Common.hlsli"

struct EquirectToCubeMapResources
{
    uint                     MipLevel;
    Texture2D                InputTexture;
    RWTexture2DArray<float4> OutputTexture;
    SamplerState             InputTextureSampler;
};

// -------------------------------------------------- Constants --------------------------------------------------- //
cbuffer EquirectToCubeMapConstants : register(b0, ATOM_INSTANCE_DATA_SPACE)
{
    uint EquirectToCubeMapResources_MipLevel;
}

// -------------------------------------------------- Resources --------------------------------------------------- //
Texture2D                EquirectToCubeMapResources_InputTexture  : register(t0, ATOM_INSTANCE_DATA_SPACE);
RWTexture2DArray<float4> EquirectToCubeMapResources_OutputTexture : register(u0, ATOM_INSTANCE_DATA_SPACE);

// --------------------------------------------------- Samplers --------------------------------------------------- //
SamplerState EquirectToCubeMapResources_InputTextureSampler : register(s0, ATOM_INSTANCE_DATA_SPACE);


EquirectToCubeMapResources CreateEquirectToCubeMapResources()
{
    EquirectToCubeMapResources resources;
    resources.MipLevel = EquirectToCubeMapResources_MipLevel;
    resources.InputTexture = EquirectToCubeMapResources_InputTexture;
    resources.OutputTexture = EquirectToCubeMapResources_OutputTexture;
    resources.InputTextureSampler = EquirectToCubeMapResources_InputTextureSampler;
    return resources;
}

#endif // __EQUIRECTTOCUBEMAPRESOURCES_HLSLI__