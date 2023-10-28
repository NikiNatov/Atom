#shadertype cs

#include "CommonConstants.hlsli"
#include "autogen/hlsl/EquirectToCubeMapParams.hlsli"

static EquirectToCubeMapParams g_EquirectToCubeMapParams = CreateEquirectToCubeMapParams();

float3 GetSamplingVector(uint3 ThreadID, float outputWidth, float outputHeight)
{
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
	float outputWidth, outputHeight, outputDepth;
    g_EquirectToCubeMapParams.OutputTexture.GetDimensions(outputWidth, outputHeight, outputDepth);

	float3 v = GetSamplingVector(ThreadID, outputWidth, outputHeight);

	float phi = atan2(v.z, v.x);
	float theta = acos(v.y);

    float4 color = g_EquirectToCubeMapParams.InputTexture.SampleLevel(g_EquirectToCubeMapParams.InputTextureSampler, float2(phi / TwoPI, theta / PI), g_EquirectToCubeMapParams.MipLevel);

    g_EquirectToCubeMapParams.OutputTexture[ThreadID] = color;
}