#shadertype vs

#include "autogen/hlsl/DefaultLayout.hlsli"
#include "autogen/hlsl/MeshDrawParams.hlsli"
#include "autogen/hlsl/FrameParams.hlsli"

// Inputs
struct VSInput
{
    float3 Position    : POSITION;
    float2 UV          : TEX_COORD;
    float3 Normal      : NORMAL;
    float3 Tangent     : TANGENT;
    float3 Bitangent   : BITANGENT;
    uint4  BoneIDs     : BONE_IDS;
    float4 BoneWeights : BONE_WEIGHTS;
};

struct VSOutput
{
    float4 PositionSV     : SV_POSITION;
    float3 Position       : POSITION;
    float2 UV             : TEX_COORD;
    float3 Normal         : NORMAL;
    float3 Tangent        : TANGENT;
    float3 Bitangent      : BITANGENT;
    float3 CameraPosition : CAMERA_POSITION;
};

static MeshDrawParams g_MeshDrawParams = CreateMeshDrawParams();
static FrameParams g_FrameParams = CreateFrameParams();

[RootSignature(DefaultLayout_Graphics)]
VSOutput VSMain(in VSInput input)
{
    float4 animatedPos = float4(0.0, 0.0, 0.0, 0.0);
    float4 animatedNormal = float4(0.0, 0.0, 0.0, 0.0);
    float4 animatedTangent = float4(0.0, 0.0, 0.0, 0.0);
    float4 animatedBitangent = float4(0.0, 0.0, 0.0, 0.0);

    for (int i = 0; i < 4; i++)
    {
        float4 posePosition = mul(g_FrameParams.BoneTransforms[g_MeshDrawParams.BoneTransformOffset + input.BoneIDs[i]], float4(input.Position, 1.0));
        animatedPos += posePosition * input.BoneWeights[i];

        float4 normal = mul(g_FrameParams.BoneTransforms[g_MeshDrawParams.BoneTransformOffset + input.BoneIDs[i]], float4(input.Normal, 0.0));
        animatedNormal += normal * input.BoneWeights[i];

        float4 tangent = mul(g_FrameParams.BoneTransforms[g_MeshDrawParams.BoneTransformOffset + input.BoneIDs[i]], float4(input.Tangent, 0.0));
        animatedTangent += tangent * input.BoneWeights[i];

        float4 bitangent = mul(g_FrameParams.BoneTransforms[g_MeshDrawParams.BoneTransformOffset + input.BoneIDs[i]], float4(input.Bitangent, 0.0));
        animatedBitangent += bitangent * input.BoneWeights[i];
    }

    VSOutput output;
    output.Position = mul(g_MeshDrawParams.Transform, animatedPos).xyz;
    output.UV = input.UV;
    output.Normal = normalize(mul((float3x3)g_MeshDrawParams.Transform, animatedNormal.xyz));
    output.Tangent = normalize(mul((float3x3)g_MeshDrawParams.Transform, animatedTangent.xyz));
    output.Bitangent = normalize(mul((float3x3)g_MeshDrawParams.Transform, animatedBitangent.xyz));
    output.CameraPosition = g_FrameParams.CameraPosition;
    output.PositionSV = mul(mul(g_FrameParams.ProjectionMatrix, g_FrameParams.ViewMatrix), float4(output.Position, 1.0));
    return output;
}

#shadertype ps

#include "PBRCommon.hlsli"

#include "autogen/hlsl/DefaultLayout.hlsli"
#include "autogen/hlsl/MaterialParams.hlsli"
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

static MaterialParams g_MaterialParams = CreateMaterialParams();
static FrameParams g_FrameParams = CreateFrameParams();

[RootSignature(DefaultLayout_Graphics)]
float4 PSMain(in PSInput input) : SV_Target
{
    float4 albedoColor = g_MaterialParams.UseAlbedoMap ? g_MaterialParams.AlbedoMap.Sample(g_MaterialParams.AlbedoMapSampler, input.UV).rgba : g_MaterialParams.AlbedoColor;
    float3 normal = g_MaterialParams.UseNormalMap ? GetNormalFromMap(g_MaterialParams.NormalMap, g_MaterialParams.NormalMapSampler, input.UV, input.Tangent, input.Bitangent, input.Normal) : input.Normal;
    float metalness = g_MaterialParams.UseMetalnessMap ? g_MaterialParams.MetalnessMap.Sample(g_MaterialParams.MetalnessMapSampler, input.UV).r : g_MaterialParams.Metalness;
    float roughness = g_MaterialParams.UseRoughnessMap ? g_MaterialParams.RoughnessMap.Sample(g_MaterialParams.RoughnessMapSampler, input.UV).r : g_MaterialParams.Roughness;

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