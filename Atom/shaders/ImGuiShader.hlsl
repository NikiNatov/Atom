#shadertype vs

#include "autogen/hlsl/DefaultLayout.hlsli"
#include "autogen/hlsl/ImGuiPassParams.hlsli"

struct VSInput
{
    float2 Position : POSITION;
    float2 UV       : TEX_COORD;
    float4 Color    : COLOR;
};

struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 UV       : TEX_COORD;
    float4 Color    : COLOR;
};

static ImGuiPassParams g_ImGuiPassParams = CreateImGuiPassParams();

[RootSignature(DefaultLayout_Graphics)]
VSOutput VSMain(VSInput input)
{
    VSOutput output;
    output.Position = mul(g_ImGuiPassParams.MVPMatrix, float4(input.Position.xy, 0.0f, 1.0f));
    output.Color = input.Color;
    output.UV = input.UV;
    return output;
}

#shadertype ps

#include "autogen/hlsl/DefaultLayout.hlsli"
#include "autogen/hlsl/ImGuiTextureParams.hlsli"

struct PSInput
{
    float4 Position : SV_POSITION;
    float2 UV       : TEX_COORD;
    float4 Color    : COLOR;
};

static DefaultLayoutStaticSamplers g_DefaultLayoutStaticSamplers = CreateDefaultLayoutStaticSamplers();
static ImGuiTextureParams g_ImGuiTextureParams = CreateImGuiTextureParams();

[RootSignature(DefaultLayout_Graphics)]
float4 PSMain(PSInput input) : SV_Target
{
    return input.Color * g_ImGuiTextureParams.Texture.Sample(g_DefaultLayoutStaticSamplers.LinearRepeatSampler, input.UV);
}