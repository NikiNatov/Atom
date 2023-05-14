#ifndef __COMPOSITERESOURCES_HLSLI__
#define __COMPOSITERESOURCES_HLSLI__

#include "include/Common.hlsli"

struct CompositeResources
{
    Texture2D    SceneTexture;
    SamplerState SceneTextureSampler;
};

// -------------------------------------------------- Resources --------------------------------------------------- //
Texture2D CompositeResources_SceneTexture : register(t0, ATOM_INSTANCE_DATA_SPACE);

// --------------------------------------------------- Samplers --------------------------------------------------- //
SamplerState CompositeResources_SceneTextureSampler: register(s0, ATOM_INSTANCE_DATA_SPACE);


CompositeResources CreateCompositeResources()
{
    CompositeResources resources;
    resources.SceneTexture = CompositeResources_SceneTexture;
    resources.SceneTextureSampler = CompositeResources_SceneTextureSampler;
    return resources;
}

#endif // __COMPOSITERESOURCES_HLSLI__