#ifndef FRAMERESOURCES_HLSLI
#define FRAMERESOURCES_HLSLI

#include "include/Common.hlsli"

struct FrameResources
{
    matrix                   ViewMatrix;
    matrix                   ProjectionMatrix;
    matrix                   InvViewProjMatrix;
    float3                   CameraPosition;
    float                    CameraExposure;
    uint                     NumLights;
    TextureCube              EnvironmentMap;
    TextureCube              IrradianceMap;
    Texture2D                BRDFMap;
    StructuredBuffer<matrix> BoneTransforms;
    StructuredBuffer<Light>  Lights;
    SamplerState             EnvironmentMapSampler;
    SamplerState             IrradianceMapSampler;
    SamplerState             BRDFMapSampler;
};

// -------------------------------------------------- Constants --------------------------------------------------- //
cbuffer FrameConstants : register(b0, ATOM_FRAME_DATA_SPACE)
{
    matrix FrameResources_ViewMatrix;
    matrix FrameResources_ProjectionMatrix;
    matrix FrameResources_InvViewProjMatrix;
    float3 FrameResources_CameraPosition;
    float  FrameResources_CameraExposure;
    uint   FrameResources_NumLights;
};

// -------------------------------------------------- Resources --------------------------------------------------- //
TextureCube              FrameResources_EnvironmentMap : register(t0, ATOM_FRAME_DATA_SPACE);
TextureCube              FrameResources_IrradianceMap  : register(t1, ATOM_FRAME_DATA_SPACE);
Texture2D                FrameResources_BRDFMap        : register(t2, ATOM_FRAME_DATA_SPACE);
StructuredBuffer<Light>  FrameResources_Lights         : register(t3, ATOM_FRAME_DATA_SPACE);
StructuredBuffer<matrix> FrameResources_BoneTransforms : register(t4, ATOM_FRAME_DATA_SPACE);

// --------------------------------------------------- Samplers --------------------------------------------------- //
SamplerState FrameResources_EnvironmentMapSampler : register(s0, ATOM_FRAME_DATA_SPACE);
SamplerState FrameResources_IrradianceMapSampler  : register(s1, ATOM_FRAME_DATA_SPACE);
SamplerState FrameResources_BRDFMapSampler        : register(s2, ATOM_FRAME_DATA_SPACE);

FrameResources CreateFrameResources()
{
    FrameResources resources;
    resources.ViewMatrix = FrameResources_ViewMatrix;
    resources.ProjectionMatrix = FrameResources_ProjectionMatrix;
    resources.InvViewProjMatrix = FrameResources_InvViewProjMatrix;
    resources.CameraPosition = FrameResources_CameraPosition;
    resources.CameraExposure = FrameResources_CameraExposure;
    resources.NumLights = FrameResources_NumLights;
    resources.EnvironmentMap = FrameResources_EnvironmentMap;
    resources.IrradianceMap = FrameResources_IrradianceMap;
    resources.BRDFMap = FrameResources_BRDFMap;
    resources.BoneTransforms = FrameResources_BoneTransforms;
    resources.Lights = FrameResources_Lights;
    resources.EnvironmentMapSampler = FrameResources_EnvironmentMapSampler;
    resources.IrradianceMapSampler = FrameResources_IrradianceMapSampler;
    resources.BRDFMapSampler = FrameResources_BRDFMapSampler;
    return resources;
}

#endif // FRAMERESOURCES_HLSLI