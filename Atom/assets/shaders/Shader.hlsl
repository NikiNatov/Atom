struct VSInput
{
    float3 Position : POSITION;
};

struct PSInput
{
    float4 PositionSV : SV_POSITION;
    float3 Position   : POSITION;
};

struct Camera
{
    row_major matrix ViewMatrix;
    row_major matrix ProjectionMatrix;
};

ConstantBuffer<Camera> CameraCB : register(b0);

PSInput VSMain(in VSInput input)
{
    PSInput output;
    output.Position = input.Position;
    output.PositionSV = mul(float4(output.Position, 1.0), mul(CameraCB.ViewMatrix, CameraCB.ProjectionMatrix));

    return output;
}

float4 PSMain(in PSInput input) : SV_TARGET
{
    return float4(input.Position + 0.5, 1.0);
}