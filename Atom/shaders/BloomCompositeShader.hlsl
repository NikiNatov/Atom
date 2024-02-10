#shadertype vs

#include "autogen/hlsl/DefaultLayout.hlsli"

struct VSInput
{
    float3 Position : POSITION;
    float2 UV : TEX_COORD;
};

struct VSOutput
{
    float4 PositionSV : SV_POSITION;
    float2 UV : TEX_COORD;
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
#include "autogen/hlsl/BloomCompositeParams.hlsli"

struct PSInput
{
    float4 PositionSV : SV_POSITION;
    float2 UV : TEX_COORD;
};

static DefaultLayoutStaticSamplers g_DefaultLayoutStaticSamplers = CreateDefaultLayoutStaticSamplers();
static BloomCompositeParams g_BloomCompositeParams = CreateBloomCompositeParams();

[RootSignature(DefaultLayout_Graphics)]
float4 PSMain(in PSInput input) : SV_Target
{
    float3 sceneColor = g_BloomCompositeParams.SceneTexture.Sample(g_DefaultLayoutStaticSamplers.LinearClampSampler, input.UV).rgb;
    float3 bloomColor = g_BloomCompositeParams.BloomTexture.Sample(g_DefaultLayoutStaticSamplers.LinearClampSampler, input.UV).rgb;

    return float4(lerp(sceneColor, bloomColor, g_BloomCompositeParams.BloomStrength), 1.0);
}