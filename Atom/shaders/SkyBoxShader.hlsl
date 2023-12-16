#shadertype vs

#include "autogen/hlsl/DefaultLayout.hlsli"
#include "autogen/hlsl/FrameParams.hlsli"

struct VSInput
{
	float3 Position : POSITION;
	float2 UV 		: TEX_COORD;
};

struct VSOutput
{
	float4 PositionSV : SV_POSITION;
	float3 Position	  : POSITION;
	float2 UV 		  : TEX_COORD;
};

static FrameParams g_FrameParams = CreateFrameParams();

[RootSignature(DefaultLayout_Graphics)]
VSOutput VSMain(in VSInput input)
{
    VSOutput output;
	float3 pos = input.Position;
	pos.z = 1.0f;
	output.PositionSV = float4(pos, 1.0);
	output.Position = mul((float3x4)g_FrameParams.InvViewProjMatrix, float4(pos, 1.0)).xyz;
	output.UV = input.UV;
	return output;
}

#shadertype ps

#include "autogen/hlsl/DefaultLayout.hlsli"
#include "autogen/hlsl/FrameParams.hlsli"

struct PSInput
{
	float4 PositionSV : SV_POSITION;
	float3 Position	  : POSITION;
	float2 UV 		  : TEX_COORD;
};

static DefaultLayoutStaticSamplers g_DefaultLayoutStaticSamplers = CreateDefaultLayoutStaticSamplers();
static FrameParams g_FrameParams = CreateFrameParams();

[RootSignature(DefaultLayout_Graphics)]
float4 PSMain(in PSInput input) : SV_TARGET
{
    return g_FrameParams.EnvironmentMap.SampleLevel(g_DefaultLayoutStaticSamplers.LinearClampSampler, input.Position, 0);
}