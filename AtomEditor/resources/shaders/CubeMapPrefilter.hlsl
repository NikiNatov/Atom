#include "include/CommonConstants.hlsli"

static const uint NumSamples = 512;
static const float InvNumSamples = 1.0 / float(NumSamples);

cbuffer SpecularMapFilterSettings : register(b0)
{
	float Roughness;
	float p0;
};

TextureCube EnvMapUnfiltered : register(t0);
SamplerState DefaultSampler : register(s0);

RWTexture2DArray<float4> EnvMap : register(u0);

float RadicalInverse_VdC(uint bits)
{
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10;
}

float2 SampleHammersley(uint i)
{
	return float2(i * InvNumSamples, RadicalInverse_VdC(i));
}

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

float3 GetSamplingVector(uint3 ThreadID)
{
	float outputWidth, outputHeight, outputDepth;
	EnvMap.GetDimensions(outputWidth, outputHeight, outputDepth);

	float2 st = ThreadID.xy / float2(outputWidth, outputHeight);
	float2 uv = 2.0 * float2(st.x, 1.0 - st.y) - 1.0;

	float3 ret = 0.0;
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

void ComputeBasisVectors(const float3 N, out float3 S, out float3 T)
{
	T = cross(N, float3(0.0, 1.0, 0.0));
	T = lerp(cross(N, float3(1.0, 0.0, 0.0)), T, step(Epsilon, dot(T, T)));

	T = normalize(T);
	S = normalize(cross(N, T));
}

float3 TangentToWorld(const float3 v, const float3 N, const float3 S, const float3 T)
{
	return S * v.x + T * v.y + N * v.z;
}

[numthreads(32, 32, 1)]
void CSMain(uint3 ThreadID : SV_DispatchThreadID)
{
	uint outputWidth, outputHeight, outputDepth;
	EnvMap.GetDimensions(outputWidth, outputHeight, outputDepth);

	if (ThreadID.x >= outputWidth || ThreadID.y >= outputHeight) 
	{
		return;
	}

	float inputWidth, inputHeight, inputLevels;
	EnvMapUnfiltered.GetDimensions(0, inputWidth, inputHeight, inputLevels);

	float wt = 4.0 * PI / (6 * inputWidth * inputHeight);

	float3 N = GetSamplingVector(ThreadID);
	float3 Lo = N;

	float3 S, T;
	ComputeBasisVectors(N, S, T);

	float3 color = 0;
	float weight = 0;

	for (uint i = 0; i < NumSamples; ++i) 
	{
		float2 u = SampleHammersley(i);
		float3 Lh = TangentToWorld(SampleGGX(u.x, u.y, Roughness), N, S, T);

		float3 Li = 2.0 * dot(Lo, Lh) * Lh - Lo;

		float cosLi = dot(N, Li);
		if (cosLi > 0.0) 
		{

			float cosLh = max(dot(N, Lh), 0.0);

			float pdf = NdfGGX(cosLh, Roughness) * 0.25;

			float ws = 1.0 / (NumSamples * pdf);

			float mipLevel = max(0.5 * log2(ws / wt) + 1.0, 0.0);

			color += EnvMapUnfiltered.SampleLevel(DefaultSampler, Li, mipLevel).rgb * cosLi;
			weight += cosLi;
		}
	}

	color /= weight;

	EnvMap[ThreadID] = float4(color, 1.0);
}