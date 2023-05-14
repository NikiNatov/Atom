#ifndef __IMGUIRESOURCES_HLSLI__
#define __IMGUIRESOURCES_HLSLI__

#include "include/Common.hlsli"

struct ImGuiResources
{
    matrix       MVPMatrix;
    Texture2D    Texture;
    SamplerState TextureSampler;
};

// -------------------------------------------------- Constants --------------------------------------------------- //
cbuffer ImGuiConstants : register(b0, ATOM_INSTANCE_DATA_SPACE)
{
    matrix ImGuiResources_MVPMatrix;
};

// -------------------------------------------------- Resources --------------------------------------------------- //
Texture2D ImGuiResources_Texture : register(t0, ATOM_INSTANCE_DATA_SPACE);

// --------------------------------------------------- Samplers --------------------------------------------------- //
SamplerState ImGuiResources_TextureSampler: register(s0, ATOM_INSTANCE_DATA_SPACE);

ImGuiResources CreateImGuiResources()
{
    ImGuiResources resources;
    resources.MVPMatrix = ImGuiResources_MVPMatrix;
    resources.Texture = ImGuiResources_Texture;
    resources.TextureSampler = ImGuiResources_TextureSampler;
    return resources;
}

#endif // __IMGUIRESOURCES_HLSLI__