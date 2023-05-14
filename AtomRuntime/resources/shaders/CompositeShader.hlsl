#include "include/CompositeResources.hlsli"
#include "include/FrameResources.hlsli"

static const float Gamma = 2.2;
static const float PureWhite = 1.0;

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

static CompositeResources g_CompositeResources = CreateCompositeResources();
static FrameResources g_FrameResources = CreateFrameResources();

PSInput VSMain(in VSInput input)
{
    PSInput output;
    output.UV = input.UV;
    output.PositionSV = float4(input.Position, 1.0);
    return output;
}

float4 PSMain(in PSInput input) : SV_Target
{
    float3 color = g_CompositeResources.SceneTexture.Sample(g_CompositeResources.SceneTextureSampler, input.UV).rgb * g_FrameResources.CameraExposure;

    float luminance = dot(color, float3(0.2126, 0.7152, 0.0722));
    float mappedLuminance = (luminance * (1.0 + luminance / (PureWhite * PureWhite))) / (1.0 + luminance);

    float3 mappedColor = (mappedLuminance / luminance) * color;

    return float4(pow(abs(mappedColor), 1.0 / Gamma), 1.0);
}