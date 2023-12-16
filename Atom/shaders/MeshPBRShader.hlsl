#shadertype vs

#include "autogen/hlsl/DefaultLayout.hlsli"
#include "autogen/hlsl/MeshDrawParams.hlsli"
#include "autogen/hlsl/FrameParams.hlsli"

// Inputs
struct VSInput
{
    float3 Position  : POSITION;
    float2 UV        : TEX_COORD;
    float3 Normal    : NORMAL;
    float3 Tangent   : TANGENT;
    float3 Bitangent : BITANGENT;
};

struct VSOutput
{
    float4 PositionSV     : SV_POSITION;
    float3 Position       : POSITION;
    float2 UV             : TEX_COORD;
    float3 Normal         : NORMAL;
    float3 Tangent        : TANGENT;
    float3 Bitangent      : BITANGENT;
};

static MeshDrawParams g_MeshDrawParams = CreateMeshDrawParams();
static FrameParams g_FrameParams = CreateFrameParams();

[RootSignature(DefaultLayout_Graphics)]
VSOutput VSMain(in VSInput input)
{
    VSOutput output;
    output.Position = mul(g_MeshDrawParams.Transform, float4(input.Position, 1.0)).xyz;
    output.UV = input.UV;
    output.Normal = normalize(mul((float3x3)g_MeshDrawParams.Transform, input.Normal));
    output.Tangent = normalize(mul((float3x3)g_MeshDrawParams.Transform, input.Tangent));
    output.Bitangent = normalize(mul((float3x3)g_MeshDrawParams.Transform, input.Bitangent));
    output.PositionSV = mul(mul(g_FrameParams.ProjectionMatrix, g_FrameParams.ViewMatrix), float4(output.Position, 1.0));
    return output;
}

#shadertype ps

#include "PBRCommon.hlsli"

#include "autogen/hlsl/DefaultLayout.hlsli"
#include "autogen/hlsl/MaterialPBRParams.hlsli"
#include "autogen/hlsl/FrameParams.hlsli"

struct PSInput
{
    float4 PositionSV : SV_POSITION;
    float3 Position   : POSITION;
    float2 UV         : TEX_COORD;
    float3 Normal     : NORMAL;
    float3 Tangent    : TANGENT;
    float3 Bitangent  : BITANGENT;
};

static MaterialPBRParams g_MaterialPBRParams = CreateMaterialPBRParams();
static FrameParams g_FrameParams = CreateFrameParams();

[RootSignature(DefaultLayout_Graphics)]
float4 PSMain(in PSInput input) : SV_Target
{
    float4 albedoColor = g_MaterialPBRParams.UseAlbedoMap ? g_MaterialPBRParams.AlbedoMap.Sample(g_MaterialPBRParams.AlbedoMapSampler, input.UV).rgba : g_MaterialPBRParams.AlbedoColor;
    float3 normal = g_MaterialPBRParams.UseNormalMap ? GetNormalFromMap(g_MaterialPBRParams.NormalMap, g_MaterialPBRParams.NormalMapSampler, input.UV, input.Tangent, input.Bitangent, input.Normal) : input.Normal;
    float metalness = g_MaterialPBRParams.UseMetalnessMap ? g_MaterialPBRParams.MetalnessMap.Sample(g_MaterialPBRParams.MetalnessMapSampler, input.UV).r : g_MaterialPBRParams.Metalness;
    float roughness = g_MaterialPBRParams.UseRoughnessMap ? g_MaterialPBRParams.RoughnessMap.Sample(g_MaterialPBRParams.RoughnessMapSampler, input.UV).r : g_MaterialPBRParams.Roughness;

    float3 N = normalize(normal);
    float3 V = normalize(g_FrameParams.CameraPosition - input.Position);
    float3 F0 = lerp(float3(0.04, 0.04, 0.04), albedoColor.rgb, metalness);

    float3 outgoingLightColor = float3(0.0, 0.0, 0.0);

    // Lights
    for (uint i = 0; i < g_FrameParams.NumLights; i++)
    {
        switch (g_FrameParams.Lights[i].LightType)
        {
            case POINT_LIGHT:
                outgoingLightColor += CalculatePointLight(g_FrameParams.Lights[i], F0, V, N, input.Position, albedoColor.rgb, metalness, roughness);
                break;
            case DIR_LIGHT:
                outgoingLightColor += CalculateDirectionalLight(g_FrameParams.Lights[i], F0, V, N, albedoColor.rgb, metalness, roughness);
                break;
            case SPOT_LIGHT:
                outgoingLightColor += CalculateSpotLight(g_FrameParams.Lights[i], F0, V, N, input.Position, albedoColor.rgb, metalness, roughness);
                break;
        }
    }

    // IBL
    float3 ambientColor = float3(0.0, 0.0, 0.0);
    {
        float3 irradianceColor = g_FrameParams.IrradianceMap.Sample(g_FrameParams.IrradianceMapSampler, N).rgb;

        float3 Ks = FresnelSchlickFunction(F0, N, V, roughness);
        float3 Kd = (float3(1.0, 1.0, 1.0) - Ks) * (1.0 - metalness);
        float3 diffuseColor = Kd * albedoColor.rgb * irradianceColor;

        float3 envSpecularColor = g_FrameParams.EnvironmentMap.SampleLevel(g_FrameParams.EnvironmentMapSampler, reflect(-V, N), roughness * 10).rgb;
        float2 specularBRDF = g_FrameParams.BRDFMap.Sample(g_FrameParams.BRDFMapSampler, float2(max(dot(N, V), 0.0), roughness)).rg;
        float3 specularColor = envSpecularColor * (Ks * specularBRDF.x + specularBRDF.y);

        ambientColor = diffuseColor + specularColor;
    }

    return float4(outgoingLightColor + ambientColor, albedoColor.a);
}