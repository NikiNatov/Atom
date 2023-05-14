#ifndef __FULLSCREENQUADRESOURCES_HLSLI__
#define __FULLSCREENQUADRESOURCES_HLSLI__

#include "include/Common.hlsli"

struct FullscreenQuadResources
{
    Texture2D    Texture;
    SamplerState TextureSampler;
};

// -------------------------------------------------- Resources --------------------------------------------------- //
Texture2D FullscreenQuadResources_Texture : register(t0, ATOM_INSTANCE_DATA_SPACE);

// --------------------------------------------------- Samplers --------------------------------------------------- //
SamplerState FullscreenQuadResources_TextureSampler: register(s0, ATOM_INSTANCE_DATA_SPACE);

FullscreenQuadResources CreateFullscreenQuadResources()
{
    FullscreenQuadResources resources;
    resources.Texture = FullscreenQuadResources_Texture;
    resources.TextureSampler = FullscreenQuadResources_TextureSampler;
    return resources;
}

#endif // __FULLSCREENQUADRESOURCES_HLSLI__