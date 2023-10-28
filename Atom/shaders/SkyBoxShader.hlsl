#shadertype vs

#include "autogen/hlsl/FrameParams.hlsli"

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

static FrameParams g_FrameParams = CreateFrameParams();

PSInput VSMain(in VSInput input)
{
	PSInput output;
	float3 pos = input.Position;
	pos.z = 1.0f;
	output.PositionSV = float4(pos, 1.0);
	output.Position = mul((float3x4)g_FrameParams.InvViewProjMatrix, float4(pos, 1.0)).xyz;
	output.UV = input.UV;
	return output;
}

#shadertype ps

#include "autogen/hlsl/FrameParams.hlsli"

struct PSInput
{
	float4 PositionSV : SV_POSITION;
	float3 Position	  : POSITION;
	float2 UV 		  : TEX_COORD;
};

static FrameParams g_FrameParams = CreateFrameParams();

float4 PSMain(in PSInput input) : SV_TARGET
{
	return g_FrameParams.EnvironmentMap.SampleLevel(g_FrameParams.EnvironmentMapSampler, input.Position, 0);
}