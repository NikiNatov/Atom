#include "include/ImGuiResources.hlsli"

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

static ImGuiResources g_ImGuiResources = CreateImGuiResources();

PSInput VSMain(VSInput input)
{
    PSInput output;
    output.Position = mul(g_ImGuiResources.MVPMatrix, float4(input.Position.xy, 0.0f, 1.0f));
    output.Color = input.Color;
    output.UV = input.UV;
    return output;
}

float4 PSMain(PSInput input) : SV_Target
{
    return input.Color * g_ImGuiResources.Texture.Sample(g_ImGuiResources.TextureSampler, input.UV);
}