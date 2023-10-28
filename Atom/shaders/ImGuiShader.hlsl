#shadertype vs

#include "autogen/hlsl/ImGuiParams.hlsli"

struct VSInput
{
    float2 Position : POSITION;
    float2 UV       : TEX_COORD;
    float4 Color    : COLOR;
};

struct PSInput
{
    float4 Position : SV_POSITION;
    float2 UV       : TEX_COORD;
    float4 Color    : COLOR;
};

static ImGuiParams g_ImGuiParams = CreateImGuiParams();

PSInput VSMain(VSInput input)
{
    PSInput output;
    output.Position = mul(g_ImGuiParams.MVPMatrix, float4(input.Position.xy, 0.0f, 1.0f));
    output.Color = input.Color;
    output.UV = input.UV;
    return output;
}

#shadertype ps

#include "autogen/hlsl/ImGuiParams.hlsli"

struct PSInput
{
    float4 Position : SV_POSITION;
    float2 UV       : TEX_COORD;
    float4 Color    : COLOR;
};

static ImGuiParams g_ImGuiParams = CreateImGuiParams();

float4 PSMain(PSInput input) : SV_Target
{
    return input.Color * g_ImGuiParams.Texture.Sample(g_ImGuiParams.TextureSampler, input.UV);
}