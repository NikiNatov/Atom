#include "include/CommonConstants.hlsli"

cbuffer EquirectToCubeMapCB : register(b0)
{
	uint MipLevel;
	float p0;
}

Texture2D InputTexture : register(t0);
SamplerState DefaultSampler : register(s0);

RWTexture2DArray<float4> OutputTexture : register(u0);

float3 GetSamplingVector(uint3 ThreadID)
{
	float outputWidth, outputHeight, outputDepth;
	OutputTexture.GetDimensions(outputWidth, outputHeight, outputDepth);

	float2 st = ThreadID.xy / float2(outputWidth, outputHeight);
	float2 uv = 2.0 * float2(st.x, 1.0 - st.y) - 1.0;

	float3 ret = float3(0.0, 0.0, 0.0);
	switch (ThreadID.z)
	{
		case 0: ret = float3(1.0, uv.y, -uv.x); break;
		case 1: ret = float3(-1.0, uv.y, uv.x); break;
		case 2: ret = float3(uv.x, 1.0, -uv.y); break;
		case 3: ret = float3(uv.x, -1.0, uv.y); break;
		case 4: ret = float3(uv.x, uv.y, 1.0); break;
		case 5: ret = float3(-uv.x, uv.y, -1.0); break;
	}
	return normalize(ret);
}

[numthreads(32, 32, 1)]
void CSMain(uint3 ThreadID : SV_DispatchThreadID)
{
	float3 v = GetSamplingVector(ThreadID);

	float phi = atan2(v.z, v.x);
	float theta = acos(v.y);

	float4 color = InputTexture.SampleLevel(DefaultSampler, float2(phi / TwoPI, theta / PI), MipLevel);

	OutputTexture[ThreadID] = color;
}