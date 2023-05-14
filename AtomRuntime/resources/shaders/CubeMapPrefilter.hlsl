#include "include/CubeMapPrefilterResources.hlsli"
#include "include/PBRCommon.hlsli"

float3 SampleGGX(float u1, float u2, float roughness)
{
	float alpha = roughness * roughness;

	float cosTheta = sqrt((1.0 - u2) / (1.0 + (alpha * alpha - 1.0) * u2));
	float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
	float phi = TwoPI * u1;

	return float3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);
}

float NdfGGX(float cosLh, float roughness)
{
	float alpha = roughness * roughness;
	float alphaSq = alpha * alpha;

	float denom = (cosLh * cosLh) * (alphaSq - 1.0) + 1.0;
	return alphaSq / (PI * denom * denom);
}

static CubeMapPrefilterResources g_CubeMapPrefilterResources = CreateCubeMapPrefilterResources();

[numthreads(32, 32, 1)]
void CSMain(uint3 ThreadID : SV_DispatchThreadID)
{
	uint outputWidth, outputHeight, outputDepth;
	g_CubeMapPrefilterResources.EnvMap.GetDimensions(outputWidth, outputHeight, outputDepth);

	if (ThreadID.x >= outputWidth || ThreadID.y >= outputHeight) 
	{
		return;
	}

	float inputWidth, inputHeight, inputLevels;
	g_CubeMapPrefilterResources.EnvMapUnfiltered.GetDimensions(0, inputWidth, inputHeight, inputLevels);

	float wt = 4.0 * PI / (6 * inputWidth * inputHeight);

	float3 N = GetSamplingVector(ThreadID, outputWidth, outputHeight);
	float3 Lo = N;

	float3 S, T;
	ComputeBasisVectors(N, S, T);

	float3 color = 0;
	float weight = 0;

	for (uint i = 0; i < NumSamples; ++i) 
	{
		float2 u = SampleHammersley(i);
		float3 Lh = TangentToWorld(SampleGGX(u.x, u.y, g_CubeMapPrefilterResources.Roughness), N, S, T);

		float3 Li = 2.0 * dot(Lo, Lh) * Lh - Lo;

		float cosLi = dot(N, Li);
		if (cosLi > 0.0) 
		{

			float cosLh = max(dot(N, Lh), 0.0);

			float pdf = NdfGGX(cosLh, g_CubeMapPrefilterResources.Roughness) * 0.25;

			float ws = 1.0 / (NumSamples * pdf);

			float mipLevel = max(0.5 * log2(ws / wt) + 1.0, 0.0);

			color += g_CubeMapPrefilterResources.EnvMapUnfiltered.SampleLevel(g_CubeMapPrefilterResources.EnvMapSampler, Li, mipLevel).rgb * cosLi;
			weight += cosLi;
		}
	}

	color /= weight;

	g_CubeMapPrefilterResources.EnvMap[ThreadID] = float4(color, 1.0);
}