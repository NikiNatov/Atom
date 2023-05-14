#include "include/CubeMapIrradianceResources.hlsli"
#include "include/PBRCommon.hlsli"

float3 SampleHemisphere(float u1, float u2)
{
	const float u1p = sqrt(max(0.0, 1.0 - u1 * u1));
	return float3(cos(TwoPI * u2) * u1p, sin(TwoPI * u2) * u1p, u1);
}

static CubeMapIrradianceResources g_CubeMapIrradianceResources = CreateCubeMapIrradianceResources();

[numthreads(32, 32, 1)]
void CSMain(uint3 ThreadID : SV_DispatchThreadID)
{
	float outputWidth, outputHeight, outputDepth;
	g_CubeMapIrradianceResources.IrradianceMap.GetDimensions(outputWidth, outputHeight, outputDepth);

	float3 N = GetSamplingVector(ThreadID, outputWidth, outputHeight);

	float3 S, T;
	ComputeBasisVectors(N, S, T);

	float3 irradiance = 0.0;
	for (uint i = 0; i < NumSamples; i++) 
	{
		float2 u = SampleHammersley(i);
		float3 Li = TangentToWorld(SampleHemisphere(u.x, u.y), N, S, T);
		float cosTheta = max(0.0, dot(Li, N));

		irradiance += 2.0 * g_CubeMapIrradianceResources.EnvMap.SampleLevel(g_CubeMapIrradianceResources.EnvMapSampler, Li, 0).rgb * cosTheta;
	}
	irradiance /= float(NumSamples);

	g_CubeMapIrradianceResources.IrradianceMap[ThreadID] = float4(irradiance, 1.0);
}