struct VSInput
{
    float3 Position : POSITION;
};

struct PSInput
{
    float4 PositionSV : SV_POSITION;
    float3 Position   : POSITION;
};

PSInput VSMain(in VSInput input)
{
    PSInput output;
    output.Position = input.Position;
    output.PositionSV = float4(input.Position, 1.0);

    return output;
}

float4 PSMain(in PSInput input) : SV_TARGET
{
    return float4(input.Position + 0.5, 1.0);
}