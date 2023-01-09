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

cbuffer SkyBoxCB : register(b0)
{
	row_major matrix InvViewProjMatrix;
	float p0;
}

PSInput VSMain(in VSInput input)
{
	PSInput output;
	float3 pos = input.Position;
	pos.z = 1.0f;
	output.PositionSV = float4(pos, 1.0);
	output.Position = mul(float4(pos, 1.0), (float4x3)InvViewProjMatrix).xyz;
	output.UV = input.UV;
	return output;
}

TextureCube EnvironmentMap : register(t0);
SamplerState EnvironmentMapSampler : register(s0);

float4 PSMain(in PSInput input) : SV_TARGET
{
	return EnvironmentMap.SampleLevel(EnvironmentMapSampler, input.Position, 0);
}