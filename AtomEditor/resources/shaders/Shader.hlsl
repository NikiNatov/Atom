struct VSInput
{
    float3 Position  : POSITION;
    float2 UV        : TEX_COORD;
    float3 Normal    : NORMAL;
    float3 Tangent   : TANGENT;
    float3 Bitangent : BITANGENT;
};

struct PSInput
{
    float4 PositionSV : SV_POSITION;
    float3 Position   : POSITION;
    float2 UV         : TEX_COORD;
    float3 Normal     : NORMAL;
    float3 Tangent    : TANGENT;
    float3 Bitangent  : BITANGENT;
};

struct Camera
{
    matrix ViewMatrix;
    matrix ProjectionMatrix;
    float3 CameraPosition;
};

ConstantBuffer<Camera> CameraCB : register(b0);

PSInput VSMain(in VSInput input)
{
    PSInput output;
    output.Position = input.Position;
    output.UV = input.UV;
    output.Normal = input.Normal;
    output.Tangent = input.Tangent;
    output.Bitangent = input.Bitangent;
    output.PositionSV = mul(float4(output.Position, 1.0), mul(CameraCB.ViewMatrix, CameraCB.ProjectionMatrix));

    return output;
}

cbuffer MaterialCB : register(b1)
{
    float4 AlbedoColor;
    bool UseAlbedoTexture;
}

Texture2D AlbedoTexture : register(t0);
SamplerState AlbedoTextureSampler : register(s0);

float4 PSMain(in PSInput input) : SV_TARGET
{
    return UseAlbedoTexture ? AlbedoTexture.Sample(AlbedoTextureSampler, input.UV) : AlbedoColor;
}