struct VSInput
{
    float3 Position : POSITION;
    float2 UV       : TEX_COORD;
};

struct PSInput
{
    float4 PositionSV : SV_POSITION;
    float3 Position   : POSITION;
    float2 UV         : TEX_COORD;
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
    output.UV = input.UV;
    output.PositionSV = mul(float4(output.Position, 1.0), mul(CameraCB.ViewMatrix, CameraCB.ProjectionMatrix));

    return output;
}

cbuffer ColorCB : register(b1)
{
    float Red;
    float Green;
    float Blue;
}

Texture2D Texture1 : register(t0);
SamplerState TextureSampler1 : register(s0);
Texture2D Texture2 : register(t1);
SamplerState TextureSampler2 : register(s1);
Texture2D Texture3 : register(t2);
SamplerState TextureSampler3 : register(s2);

float4 PSMain(in PSInput input) : SV_TARGET
{
    float4 color1 = Texture1.Sample(TextureSampler1, input.UV);
    float4 color2 = Texture2.Sample(TextureSampler2, input.UV);
    float4 color3 = Texture3.Sample(TextureSampler3, input.UV);
    return  (color1 + color2 + color3) / 3.0f;
}