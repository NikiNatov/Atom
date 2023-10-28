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
#include "autogen/hlsl/FrameParams.hlsli"

static const float Gamma = 2.2;
static const float PureWhite = 1.0;

struct PSInput
{
    float4 PositionSV : SV_POSITION;
    float2 UV         : TEX_COORD;
};

static FullscreenQuadParams g_FullscreenQuadParams = CreateFullscreenQuadParams();
static FrameParams g_FrameParams = CreateFrameParams();

float4 PSMain(in PSInput input) : SV_Target
{
    float3 color = g_FullscreenQuadParams.Texture.Sample(g_FullscreenQuadParams.TextureSampler, input.UV).rgb * g_FrameParams.CameraExposure;

    float luminance = dot(color, float3(0.2126, 0.7152, 0.0722));
    float mappedLuminance = (luminance * (1.0 + luminance / (PureWhite * PureWhite))) / (1.0 + luminance);

    float3 mappedColor = (mappedLuminance / luminance) * color;

    return float4(pow(abs(mappedColor), 1.0 / Gamma), 1.0);
}