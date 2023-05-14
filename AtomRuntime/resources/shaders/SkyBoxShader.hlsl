#include "include/FrameResources.hlsli"

struct VSInput
{
	float3 Position : POSITION;
	float2 UV 		: TEX_COORD;
};

struct PSInput
{
	float4 PositionSV : SV_POSITION;
	float3 Position	  : POSITION;
	float2 UV 		  : TEX_COORD;
};

static FrameResources g_FrameResources = CreateFrameResources();

PSInput VSMain(in VSInput input)
{
	PSInput output;
	float3 pos = input.Position;
	pos.z = 1.0f;
	output.PositionSV = float4(pos, 1.0);
	output.Position = mul((float3x4)g_FrameResources.InvViewProjMatrix, float4(pos, 1.0)).xyz;
	output.UV = input.UV;
	return output;
}

float4 PSMain(in PSInput input) : SV_TARGET
{
	return g_FrameResources.EnvironmentMap.SampleLevel(g_FrameResources.EnvironmentMapSampler, input.Position, 0);
}