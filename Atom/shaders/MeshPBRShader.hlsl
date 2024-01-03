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
    float3 ViewPosition   : VIEW_POSITION;
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
    output.ViewPosition = mul(g_FrameParams.ViewMatrix, float4(output.Position, 1.0)).xyz;
    output.PositionSV = mul(mul(g_FrameParams.ProjectionMatrix, g_FrameParams.ViewMatrix), float4(output.Position, 1.0));
    return output;
}

#shadertype ps

#include "PBRCommon.hlsli"

#include "autogen/hlsl/DefaultLayout.hlsli"
#include "autogen/hlsl/MaterialParams.hlsli"
#include "autogen/hlsl/FrameParams.hlsli"
#include "autogen/hlsl/ShadowParams.hlsli"

#define DEBUG_SHADOW_CASCADES 0

struct PSInput
{
    float4 PositionSV   : SV_POSITION;
    float3 ViewPosition : VIEW_POSITION;
    float3 Position     : POSITION;
    float2 UV           : TEX_COORD;
    float3 Normal       : NORMAL;
    float3 Tangent      : TANGENT;
    float3 Bitangent    : BITANGENT;
};

static DefaultLayoutStaticSamplers g_DefaultLayoutStaticSamplers = CreateDefaultLayoutStaticSamplers();
static MaterialParams g_MaterialParams = CreateMaterialParams();
static FrameParams g_FrameParams = CreateFrameParams();
static ShadowParams g_ShadowParams = CreateShadowParams();

float CalculateShadows(float3 worldPosition, uint cascadeIndex)
{
    float4 lightSpacePosition = mul(g_FrameParams.ShadowCascades[cascadeIndex].SunLightViewProjMatrix, float4(worldPosition, 1.0));
    lightSpacePosition.xyz /= lightSpacePosition.w;
    
    float2 shadowMapTexCoords = lightSpacePosition.xy * 0.5 + 0.5;
    shadowMapTexCoords.y = 1.0 - shadowMapTexCoords.y;
    
    float shadowMapWidth, shadowMapHeight, shadowMapArraySize;
    g_ShadowParams.CascadeShadowMap.GetDimensions(shadowMapWidth, shadowMapHeight, shadowMapArraySize);
    
    // Apply 3x3 PCF
    float2 texelUnits = 1.0f / float2(shadowMapWidth, shadowMapHeight);
    float shadow = 0.0f;
    
    for (int i = -1; i <= 1; i++)
    {
        for (int j = -1; j <= 1; j++)
        {
            float3 sampleCoords = float3(shadowMapTexCoords.x + j * texelUnits.x, shadowMapTexCoords.y + i * texelUnits.y, cascadeIndex);
            float shadowMapDepth = g_ShadowParams.CascadeShadowMap.Sample(g_DefaultLayoutStaticSamplers.NearestClampSampler, sampleCoords);
            shadow += (shadowMapDepth < lightSpacePosition.z - g_ShadowParams.DepthBias) ? 1.0f : 0.0f;
        }

    }
    
    return shadow / 9.0f;
}

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
    
    // Calculate shadows
    uint cascadeIndex = g_FrameParams.NumShadowCascades - 1;
    for (uint j = 0; j < g_FrameParams.NumShadowCascades; j++)
    {
        if (abs(input.ViewPosition.z) < g_FrameParams.ShadowCascades[j].SplitDistance)
        {
            cascadeIndex = j;
            break;
        }
    }
    
    float shadowFactor = CalculateShadows(input.Position, cascadeIndex);
    outgoingLightColor *= (1.0f - shadowFactor);

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
    
    float3 shadowCascadeColor = float3(1.0, 1.0, 1.0);
#if DEBUG_SHADOW_CASCADES
    switch (cascadeIndex)
    {
        case 0:
            shadowCascadeColor = float3(1.0, 0.0, 0.0);
            break;
        case 1:
            shadowCascadeColor = float3(0.0, 1.0, 0.0);
            break;
        case 2:
            shadowCascadeColor = float3(0.0, 0.0, 1.0);
            break;
        case 3:
            shadowCascadeColor = float3(1.0, 1.0, 0.0);
            break;
    }
#endif

    return float4((outgoingLightColor + ambientColor) * shadowCascadeColor, albedoColor.a);
}