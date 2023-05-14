#include "include/FullscreenQuadResources.hlsli"

struct VSInput
{
    float3 Position : POSITION;
    float2 UV       : TEX_COORD;
};

struct PSInput
{
    float4 PositionSV : SV_POSITION;
    float2 UV         : TEX_COORD;
};

static FullscreenQuadResources g_FullscreenQuadResources = CreateFullscreenQuadResources();

PSInput VSMain(in VSInput input)
{
    PSInput output;
    output.UV = input.UV;
    output.PositionSV = float4(input.Position, 1.0);
    return output;
}

float4 PSMain(in PSInput input) : SV_Target
{
    return g_FullscreenQuadResources.Texture.Sample(g_FullscreenQuadResources.TextureSampler, input.UV);
}