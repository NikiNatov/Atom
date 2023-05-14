#ifndef __GENERATEMIPSRESOURCES_HLSLI__
#define __GENERATEMIPSRESOURCES_HLSLI__

#include "include/Common.hlsli"



struct GenerateMipsResources
{
    float2              TexelSize;
    uint                TopMipLevel;
    Texture2D<float4>   SrcTexture;
    RWTexture2D<float4> DstTexture;
    SamplerState        BilinearClamp;
};

// -------------------------------------------------- Constants --------------------------------------------------- //
cbuffer GenerateMipsConstants : register(b0, ATOM_INSTANCE_DATA_SPACE)
{
    float2 GenerateMipsResources_TexelSize;
    uint   GenerateMipsResources_TopMipLevel;
}

// -------------------------------------------------- Resources --------------------------------------------------- //
Texture2D<float4>   GenerateMipsResources_SrcTexture : register(t0, ATOM_INSTANCE_DATA_SPACE);
RWTexture2D<float4> GenerateMipsResources_DstTexture : register(u0, ATOM_INSTANCE_DATA_SPACE);

// --------------------------------------------------- Samplers --------------------------------------------------- //
SamplerState GenerateMipsResources_BilinearClamp : register(s0, ATOM_INSTANCE_DATA_SPACE);

GenerateMipsResources CreateGenerateMipsResources()
{
    GenerateMipsResources resources;
    resources.TexelSize = GenerateMipsResources_TexelSize;
    resources.TopMipLevel = GenerateMipsResources_TopMipLevel;
    resources.SrcTexture = GenerateMipsResources_SrcTexture;
    resources.DstTexture = GenerateMipsResources_DstTexture;
    resources.BilinearClamp = GenerateMipsResources_BilinearClamp;
    return resources;
}

#endif // __GENERATEMIPSRESOURCES_HLSLI__