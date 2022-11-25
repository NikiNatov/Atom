#include "include/CommonConstants.hlsli"

struct VSInput
{
    float3 Position  : POSITION;
    float2 UV        : TEX_COORD;
    float3 Normal    : NORMAL;
    float3 Tangent   : TANGENT;
    float3 Bitangent : BITANGENT;
};

struct PSInput
{
    float4 PositionSV     : SV_POSITION;
    float3 Position       : POSITION;
    float2 UV             : TEX_COORD;
    float3 Normal         : NORMAL;
    float3 Tangent        : TANGENT;
    float3 Bitangent      : BITANGENT;
    float3 CameraPosition : CAMERA_POSITION;
};

struct Camera
{
    row_major matrix ViewMatrix;
    row_major matrix ProjectionMatrix;
    float3 CameraPosition;
};

ConstantBuffer<Camera> CameraCB : register(b0);

cbuffer TransformCB : register(b1)
{
    row_major matrix Transform;
    float p0;
}

PSInput VSMain(in VSInput input)
{
    PSInput output;
    output.Position = mul(float4(input.Position, 1.0), Transform).xyz;
    output.UV = input.UV;
    output.Normal = normalize(mul(input.Normal, (float3x3)Transform));
    output.Tangent = normalize(mul(input.Tangent, (float3x3)Transform));
    output.Bitangent = normalize(mul(input.Bitangent, (float3x3)Transform));
    output.CameraPosition = CameraCB.CameraPosition;
    output.PositionSV = mul(float4(output.Position, 1.0), mul(CameraCB.ViewMatrix, CameraCB.ProjectionMatrix));
    return output;
}

#define DIR_LIGHT   0
#define POINT_LIGHT 1
#define SPOT_LIGHT  2

struct Light
{
    float4 Position;
    float4 Direction;
    float4 Color;
    float  Intensity;
    float  ConeAngle;
    float3 AttenuationFactors;
    uint   LightType;
};

cbuffer MaterialCB : register(b2)
{
    float4 AlbedoColor;
    bool UseAlbedoMap;

    bool UseNormalMap;

    float Metalness;
    bool UseMetalnessMap;

    float Roughness;
    bool UseRoughnessMap;
}

cbuffer LightPropertiesCB : register(b3)
{
    uint NumLights;
    float p1;
}

Texture2D AlbedoMap : register(t0);
SamplerState AlbedoMapSampler: register(s0);

Texture2D NormalMap : register(t1);
SamplerState NormalMapSampler: register(s1);

Texture2D MetalnessMap : register(t2);
SamplerState MetalnessMapSampler: register(s2);

Texture2D RoughnessMap : register(t3);
SamplerState RoughnessMapSampler: register(s3);

TextureCube EnvironmentMap : register(t4);
SamplerState EnvironmentMapSampler : register(s4);

TextureCube IrradianceMap : register(t5);
SamplerState IrradianceMapSampler : register(s5);

Texture2D BRDFMap : register(t6);
SamplerState BRDFMapSampler: register(s6);

StructuredBuffer<Light> LightsBuffer : register(t7);

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
float3 FresnelSchlickFunction(float3 F0, float3 V, float3 H)
{
    float VDotH = max(dot(V, H), 0.0);
    return F0 + (float3(1.0, 1.0, 1.0) - F0) * pow(1.0 - VDotH, 5.0);
}

//Cook-Torrance equation
float3 CookTorranceFunction(float roughness, float3 F0, float3 N, float3 V, float3 L, float3 H)
{
    float alpha = roughness * roughness;

    float3 numerator = NormalDistributionFunction(alpha, N, H) * GeometryShadowingFunction(alpha, N, V, L) * FresnelSchlickFunction(F0, V, H);
    float denominator = max(4.0 * max(dot(V, N), 0.0) * max(dot(L, N), 0.0), Epsilon);

    return numerator / denominator;
}

float3 GetNormalFromMap(float2 uv, float3 tangent, float3 bitangent, float3 normal)
{
    float3x3 toWorld = float3x3(normalize(tangent), normalize(bitangent), normalize(normal));
    float3 normalMap = NormalMap.Sample(NormalMapSampler, uv).rgb * 2.0 - 1.0;
    normalMap = mul(normalMap, toWorld);
    normalMap = normalize(normalMap);
    return normalMap;
}

float3 CalculateDirectionalLight(Light light, float3 F0, float3 V, float3 N, float3 albedoColor, float metalness, float roughness)
{
    float3 L = normalize(-light.Direction.xyz);
    float3 H = normalize(V + L);

    float3 Ks = FresnelSchlickFunction(F0, V, H);
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

    float3 Ks = FresnelSchlickFunction(F0, V, H);
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

    float3 Ks = FresnelSchlickFunction(F0, V, H);
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

float4 PSMain(in PSInput input) : SV_Target
{
    float4 albedoColor = UseAlbedoMap ? AlbedoMap.Sample(AlbedoMapSampler, input.UV).rgba : AlbedoColor;
    float3 normal = UseNormalMap ? GetNormalFromMap(input.UV, input.Tangent, input.Bitangent, input.Normal) : input.Normal;
    float  metalness = UseMetalnessMap ? MetalnessMap.Sample(MetalnessMapSampler, input.UV).r : Metalness;
    float  roughness = UseRoughnessMap ? RoughnessMap.Sample(RoughnessMapSampler, input.UV).r : Roughness;

    float3 N = normalize(normal);
    float3 V = normalize(input.CameraPosition - input.Position);
    float3 F0 = lerp(float3(0.04, 0.04, 0.04), albedoColor.rgb, metalness);

    float3 outgoingLightColor = float3(0.0, 0.0, 0.0);

    // Lights
    for (uint i = 0; i < NumLights; i++)
    {
        switch (LightsBuffer[i].LightType)
        {
        case POINT_LIGHT:
            outgoingLightColor += CalculatePointLight(LightsBuffer[i], F0, V, N, input.Position, albedoColor.rgb, metalness, roughness);
            break;
        case DIR_LIGHT:
            outgoingLightColor += CalculateDirectionalLight(LightsBuffer[i], F0, V, N, albedoColor.rgb, metalness, roughness);
            break;
        case SPOT_LIGHT:
            outgoingLightColor += CalculateSpotLight(LightsBuffer[i], F0, V, N, input.Position, albedoColor.rgb, metalness, roughness);
            break;
        }
    }

    // IBL
    float3 ambientColor = float3(0.0, 0.0, 0.0);
    {
        float3 irradianceColor = IrradianceMap.Sample(IrradianceMapSampler, N).rgb;

        float3 Ks = FresnelSchlickFunction(F0, N, V);
        float3 Kd = (float3(1.0, 1.0, 1.0) - Ks) * (1.0 - metalness);
        float3 diffuseColor = Kd * albedoColor.rgb * irradianceColor;

        float3 envSpecularColor = EnvironmentMap.SampleLevel(EnvironmentMapSampler, reflect(-V, N), roughness * 10).rgb;
        float2 specularBRDF = BRDFMap.Sample(BRDFMapSampler, float2(max(dot(N, V), 0.0), roughness)).rg;
        float3 specularColor = envSpecularColor * (Ks * specularBRDF.x + specularBRDF.y);

        ambientColor = diffuseColor + specularColor;
    }

    return float4(outgoingLightColor + ambientColor, albedoColor.a);
}