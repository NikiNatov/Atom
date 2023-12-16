#shadertype vs

#include "autogen/hlsl/DefaultLayout.hlsli"

struct VSInput
{
    float3 Position : POSITION;
    float2 UV       : TEX_COORD;
};

struct VSOutput
{
    float4 PositionSV : SV_POSITION;
    float2 UV         : TEX_COORD;
};

[RootSignature(DefaultLayout_Graphics)]
VSOutput VSMain(in VSInput input)
{
    VSOutput output;
    output.UV = input.UV;
    output.PositionSV = float4(input.Position, 1.0);
    return output;
}

#shadertype ps

#include "autogen/hlsl/DefaultLayout.hlsli"
#include "autogen/hlsl/FullscreenQuadParams.hlsli"

struct PSInput
{
    float4 PositionSV : SV_POSITION;
    float2 UV         : TEX_COORD;
};

static DefaultLayoutStaticSamplers g_DefaultLayoutStaticSamplers = CreateDefaultLayoutStaticSamplers();
static FullscreenQuadParams g_FullscreenQuadParams = CreateFullscreenQuadParams();

[RootSignature(DefaultLayout_Graphics)]
float4 PSMain(in PSInput input) : SV_Target
{
    return g_FullscreenQuadParams.Texture.Sample(g_DefaultLayoutStaticSamplers.LinearClampSampler, input.UV);
}