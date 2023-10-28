#shadertype vs

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

PSInput VSMain(in VSInput input)
{
    PSInput output;
    output.UV = input.UV;
    output.PositionSV = float4(input.Position, 1.0);
    return output;
}

#shadertype ps

#include "autogen/hlsl/FullscreenQuadParams.hlsli"

struct PSInput
{
    float4 PositionSV : SV_POSITION;
    float2 UV         : TEX_COORD;
};

static FullscreenQuadParams g_FullscreenQuadParams = CreateFullscreenQuadParams();

float4 PSMain(in PSInput input) : SV_Target
{
    return g_FullscreenQuadParams.Texture.Sample(g_FullscreenQuadParams.TextureSampler, input.UV);
}