#shadertype cs

#include "PBRCommon.hlsli"
#include "autogen/hlsl/DefaultLayout.hlsli"
#include "autogen/hlsl/BRDFParams.hlsli"

float3 SampleGGX(float u1, float u2, float roughness)
{
	float alpha = roughness * roughness;

	float cosTheta = sqrt((1.0 - u2) / (1.0 + (alpha * alpha - 1.0) * u2));
	float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
	float phi = TwoPI * u1;

	return float3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);
}

float GaSchlickG1(float cosTheta, float k)
{
	return cosTheta / (cosTheta * (1.0 - k) + k);
}

float GaSchlickGGX_IBL(float cosLi, float cosLo, float roughness)
{
	float r = roughness;
	float k = (r * r) / 2.0;
	return GaSchlickG1(cosLi, k) * GaSchlickG1(cosLo, k);
}

static BRDFParams g_BRDFParams = CreateBRDFParams();

[RootSignature(DefaultLayout_Compute)]
[numthreads(32, 32, 1)]
void CSMain(uint2 ThreadID : SV_DispatchThreadID)
{
	float outputWidth, outputHeight;
    g_BRDFParams.BRDFTexture.GetDimensions(outputWidth, outputHeight);

	float cosLo = ThreadID.x / outputWidth;
	float roughness = ThreadID.y / outputHeight;

	cosLo = max(cosLo, Epsilon);

	float3 Lo = float3(sqrt(1.0 - cosLo * cosLo), 0.0, cosLo);

	float DFG1 = 0;
	float DFG2 = 0;

	for (uint i = 0; i < NumSamples; i++) 
	{
		float2 u = SampleHammersley(i);
		float3 Lh = SampleGGX(u.x, u.y, roughness);
		float3 Li = 2.0 * dot(Lo, Lh) * Lh - Lo;

		float cosLi = Li.z;
		float cosLh = Lh.z;
		float cosLoLh = max(dot(Lo, Lh), 0.0);

		if (cosLi > 0.0) 
		{
			float G = GaSchlickGGX_IBL(cosLi, cosLo, roughness);
			float Gv = G * cosLoLh / (cosLh * cosLo);
			float Fc = pow(1.0 - cosLoLh, 5);

			DFG1 += (1 - Fc) * Gv;
			DFG2 += Fc * Gv;
		}
	}

    g_BRDFParams.BRDFTexture[ThreadID] = float2(DFG1, DFG2) * InvNumSamples;
}