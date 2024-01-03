#shadertype vs

#include "autogen/hlsl/DefaultLayout.hlsli"
#include "autogen/hlsl/MeshDrawParams.hlsli"
#include "autogen/hlsl/FrameParams.hlsli"
#include "autogen/hlsl/ShadowDepthPassParams.hlsli"

// Inputs
struct VSInput
{
    float3 Position : POSITION;
    float2 UV : TEX_COORD;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 Bitangent : BITANGENT;
};

struct VSOutput
{
    float4 PositionSV : SV_POSITION;
};

static MeshDrawParams g_MeshDrawParams = CreateMeshDrawParams();
static FrameParams g_FrameParams = CreateFrameParams();
static ShadowDepthPassParams g_ShadowDepthPassParams = CreateShadowDepthPassParams();

[RootSignature(DefaultLayout_Graphics)]
VSOutput VSMain(in VSInput input)
{
    float4 worldPosition = mul(g_MeshDrawParams.Transform, float4(input.Position, 1.0));
    
    VSOutput output;
    output.PositionSV = mul(g_FrameParams.ShadowCascades[g_ShadowDepthPassParams.CascadeIndex].SunLightViewProjMatrix, worldPosition);
    
    return output;
}