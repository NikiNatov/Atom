#ifndef __MESHPBRCOMMON_HLSLI__
#define __MESHPBRCOMMON_HLSLI__

#include "include/Common.hlsli"

#define DIR_LIGHT   0
#define POINT_LIGHT 1
#define SPOT_LIGHT  2

static const uint NumSamples = 1024;
static const float InvNumSamples = 1.0 / float(NumSamples);

// --------------------------------------------------- Functions --------------------------------------------------- //
//GGX/Trowbridge-Reitz normal distribution function
float NormalDistributionFunction(float alpha, float3 N, float3 H)
{
    float NDotH = max(dot(N, H), 0.0);

    float numerator = alpha * alpha;
    float denominator = max(PI * pow(pow(NDotH, 2.0) * (pow(alpha, 2.0) - 1.0) + 1.0, 2.0), Epsilon);

    return numerator / denominator;
}

//Schlick-GGX term
float SchlickG1(float k, float3 N, float3 X)
{
    float NDotX = max(dot(N, X), 0.0);

    float numerator = NDotX;
    float denominator = max(NDotX * (1.0 - k) + k, Epsilon);

    return numerator / denominator;
}

//Schlick-GGX geometry shadowing function
float GeometryShadowingFunction(float alpha, float3 N, float3 L, float3 V)
{
    float k = alpha / 2.0;
    return SchlickG1(k, N, V) * SchlickG1(k, N, L);
}

//Fresnel-Schlick equation
float3 FresnelSchlickFunction(float3 F0, float3 V, float3 H, float roughness)
{
    float VDotH = max(dot(V, H), 0.0);
    return F0 + (max(float3(1.0 - roughness, 1.0 - roughness, 1.0 - roughness), F0) - F0) * pow(1.0 - VDotH, 5.0);
}

//Cook-Torrance equation
float3 CookTorranceFunction(float roughness, float3 F0, float3 N, float3 V, float3 L, float3 H)
{
    float alpha = roughness * roughness;

    float3 numerator = NormalDistributionFunction(alpha, N, H) * GeometryShadowingFunction(alpha, N, V, L) * FresnelSchlickFunction(F0, V, H, roughness);
    float denominator = max(4.0 * max(dot(V, N), 0.0) * max(dot(L, N), 0.0), Epsilon);

    return numerator / denominator;
}

float3 GetNormalFromMap(Texture2D normalMap, SamplerState mapSampler, float2 uv, float3 tangent, float3 bitangent, float3 normal)
{
    float3x3 toWorld = float3x3(normalize(tangent), normalize(bitangent), normalize(normal));
    float3 normalMapValue = normalMap.Sample(mapSampler, uv).rgb * 2.0 - 1.0;
    normalMapValue = mul(normalMapValue, toWorld);
    normalMapValue = normalize(normalMapValue);
    return normalMapValue;
}

float3 CalculateDirectionalLight(Light light, float3 F0, float3 V, float3 N, float3 albedoColor, float metalness, float roughness)
{
    float3 L = normalize(-light.Direction.xyz);
    float3 H = normalize(V + L);

    float3 Ks = FresnelSchlickFunction(F0, V, H, roughness);
    float3 Kd = (float3(1.0, 1.0, 1.0) - Ks) * (1.0 - metalness);

    float3 specularColor = CookTorranceFunction(roughness, F0, N, V, L, H);
    float3 brdf = Kd * albedoColor + specularColor;

    return brdf * light.Color.rgb * light.Intensity * max(dot(L, N), 0.0);
}

float3 CalculatePointLight(Light light, float3 F0, float3 V, float3 N, float3 fragmentPos, float3 albedoColor, float metalness, float roughness)
{
    float3 L = light.Position.xyz - fragmentPos;
    float distance = length(L);
    L = normalize(L);
    float3 H = normalize(V + L);

    float3 Ks = FresnelSchlickFunction(F0, V, H, roughness);
    float3 Kd = (float3(1.0, 1.0, 1.0) - Ks) * (1.0 - metalness);

    float attenuation = 1.0f / max(light.AttenuationFactors[0] + light.AttenuationFactors[1] * distance + light.AttenuationFactors[2] * distance * distance, Epsilon);
    float3 specularColor = CookTorranceFunction(roughness, F0, N, V, L, H);
    float3 brdf = Kd * albedoColor + specularColor;

    return brdf * light.Color.rgb * light.Intensity * attenuation * max(dot(L, N), 0.0);
}

float3 CalculateSpotLight(Light light, float3 F0, float3 V, float3 N, float3 fragmentPos, float3 albedoColor, float metalness, float roughness)
{
    float3 L = light.Position.xyz - fragmentPos;
    float distance = length(L);
    L = normalize(L);
    float3 H = normalize(V + L);

    float3 Ks = FresnelSchlickFunction(F0, V, H, roughness);
    float3 Kd = (float3(1.0, 1.0, 1.0) - Ks) * (1.0 - metalness);

    // Calculate spot intensity
    float minCos = cos(light.ConeAngle);
    float maxCos = (minCos + 1.0f) / 2.0f;
    float cosAngle = dot(light.Direction.xyz, -L);
    float spotIntensity = smoothstep(minCos, maxCos, cosAngle);

    float attenuation = 1.0f / max(light.AttenuationFactors[0] + light.AttenuationFactors[1] * distance + light.AttenuationFactors[2] * distance * distance, Epsilon);
    float3 specularColor = CookTorranceFunction(roughness, F0, N, V, L, H);
    float3 brdf = Kd * albedoColor + specularColor;

    return brdf * light.Color.rgb * light.Intensity * spotIntensity * attenuation * max(dot(L, N), 0.0);
}

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

#endif // __MESHPBRCOMMON_HLSLI__