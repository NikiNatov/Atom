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

Texture2D SceneTexture : register(t0);
SamplerState Sampler: register(s0);

float4 PSMain(in PSInput input) : SV_Target
{
    return SceneTexture.Sample(Sampler, input.UV);
}