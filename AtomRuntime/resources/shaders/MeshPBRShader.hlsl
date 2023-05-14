#include "include/FrameResources.hlsli"
#include "include/MeshPBRResources.hlsli"
#include "include/PBRCommon.hlsli"

// Inputs
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
};

static MeshPBRResources g_MeshPBRResources = CreateMeshPBRResources();
static FrameResources g_FrameResources = CreateFrameResources();

PSInput VSMain(in VSInput input)
{
    PSInput output;
    output.Position = mul(g_MeshPBRResources.Transform, float4(input.Position, 1.0)).xyz;
    output.UV = input.UV;
    output.Normal = normalize(mul((float3x3)g_MeshPBRResources.Transform, input.Normal));
    output.Tangent = normalize(mul((float3x3)g_MeshPBRResources.Transform, input.Tangent));
    output.Bitangent = normalize(mul((float3x3)g_MeshPBRResources.Transform, input.Bitangent));
    output.PositionSV = mul(mul(g_FrameResources.ProjectionMatrix, g_FrameResources.ViewMatrix), float4(output.Position, 1.0));
    return output;
}

float4 PSMain(in PSInput input) : SV_Target
{
    float4 albedoColor = g_MeshPBRResources.UseAlbedoMap ? g_MeshPBRResources.AlbedoMap.Sample(g_MeshPBRResources.AlbedoMapSampler, input.UV).rgba : g_MeshPBRResources.AlbedoColor;
    float3 normal = g_MeshPBRResources.UseNormalMap ? GetNormalFromMap(g_MeshPBRResources.NormalMap, g_MeshPBRResources.NormalMapSampler, input.UV, input.Tangent, input.Bitangent, input.Normal) : input.Normal;
    float  metalness = g_MeshPBRResources.UseMetalnessMap ? g_MeshPBRResources.MetalnessMap.Sample(g_MeshPBRResources.MetalnessMapSampler, input.UV).r : g_MeshPBRResources.Metalness;
    float  roughness = g_MeshPBRResources.UseRoughnessMap ? g_MeshPBRResources.RoughnessMap.Sample(g_MeshPBRResources.RoughnessMapSampler, input.UV).r : g_MeshPBRResources.Roughness;

    float3 N = normalize(normal);
    float3 V = normalize(g_FrameResources.CameraPosition - input.Position);
    float3 F0 = lerp(float3(0.04, 0.04, 0.04), albedoColor.rgb, metalness);

    float3 outgoingLightColor = float3(0.0, 0.0, 0.0);

    // Lights
    for (uint i = 0; i < g_FrameResources.NumLights; i++)
    {
        switch (g_FrameResources.Lights[i].LightType)
        {
        case POINT_LIGHT:
            outgoingLightColor += CalculatePointLight(g_FrameResources.Lights[i], F0, V, N, input.Position, albedoColor.rgb, metalness, roughness);
            break;
        case DIR_LIGHT:
            outgoingLightColor += CalculateDirectionalLight(g_FrameResources.Lights[i], F0, V, N, albedoColor.rgb, metalness, roughness);
            break;
        case SPOT_LIGHT:
            outgoingLightColor += CalculateSpotLight(g_FrameResources.Lights[i], F0, V, N, input.Position, albedoColor.rgb, metalness, roughness);
            break;
        }
    }

    // IBL
    float3 ambientColor = float3(0.0, 0.0, 0.0);
    {
        float3 irradianceColor = g_FrameResources.IrradianceMap.Sample(g_FrameResources.IrradianceMapSampler, N).rgb;

        float3 Ks = FresnelSchlickFunction(F0, N, V, roughness);
        float3 Kd = (float3(1.0, 1.0, 1.0) - Ks) * (1.0 - metalness);
        float3 diffuseColor = Kd * albedoColor.rgb * irradianceColor;

        float3 envSpecularColor = g_FrameResources.EnvironmentMap.SampleLevel(g_FrameResources.EnvironmentMapSampler, reflect(-V, N), roughness * 10).rgb;
        float2 specularBRDF = g_FrameResources.BRDFMap.Sample(g_FrameResources.BRDFMapSampler, float2(max(dot(N, V), 0.0), roughness)).rg;
        float3 specularColor = envSpecularColor * (Ks * specularBRDF.x + specularBRDF.y);

        ambientColor = diffuseColor + specularColor;
    }

    return float4(outgoingLightColor + ambientColor, albedoColor.a);
}