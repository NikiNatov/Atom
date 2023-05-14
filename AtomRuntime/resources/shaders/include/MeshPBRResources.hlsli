#ifndef MESHPBRRESOURCES_HLSLI
#define MESHPBRRESOURCES_HLSLI

#include "include/Common.hlsli"

struct MeshPBRResources
{
    matrix       Transform;
    uint         BoneTransformOffset;
    float4       AlbedoColor;
    float        Metalness;
    float        Roughness;
    int          UseAlbedoMap;
    int          UseNormalMap;
    int          UseMetalnessMap;
    int          UseRoughnessMap;
    Texture2D    AlbedoMap;
    Texture2D    NormalMap;
    Texture2D    MetalnessMap;
    Texture2D    RoughnessMap;
    SamplerState AlbedoMapSampler;
    SamplerState NormalMapSampler;
    SamplerState MetalnessMapSampler;
    SamplerState RoughnessMapSampler;
};

// -------------------------------------------------- Constants --------------------------------------------------- //
cbuffer InstanceConstants : register(b0, ATOM_INSTANCE_DATA_SPACE)
{
    matrix MeshPBRResources_Transform;
    uint   MeshPBRResources_BoneTransformOffset;
}

cbuffer MaterialConstants : register(b0, ATOM_MATERIAL_DATA_SPACE)
{
    float4 MeshPBRResources_AlbedoColor;
    float  MeshPBRResources_Metalness;
    float  MeshPBRResources_Roughness;
    int    MeshPBRResources_UseAlbedoMap;
    int    MeshPBRResources_UseNormalMap;
    int    MeshPBRResources_UseMetalnessMap;
    int    MeshPBRResources_UseRoughnessMap;
};

// -------------------------------------------------- Resources --------------------------------------------------- //
Texture2D MeshPBRResources_AlbedoMap    : register(t0, ATOM_MATERIAL_DATA_SPACE);
Texture2D MeshPBRResources_NormalMap    : register(t1, ATOM_MATERIAL_DATA_SPACE);
Texture2D MeshPBRResources_MetalnessMap : register(t2, ATOM_MATERIAL_DATA_SPACE);
Texture2D MeshPBRResources_RoughnessMap : register(t3, ATOM_MATERIAL_DATA_SPACE);

// --------------------------------------------------- Samplers --------------------------------------------------- //
SamplerState MeshPBRResources_AlbedoMapSampler    : register(s0, ATOM_MATERIAL_DATA_SPACE);
SamplerState MeshPBRResources_NormalMapSampler    : register(s1, ATOM_MATERIAL_DATA_SPACE);
SamplerState MeshPBRResources_MetalnessMapSampler : register(s2, ATOM_MATERIAL_DATA_SPACE);
SamplerState MeshPBRResources_RoughnessMapSampler : register(s3, ATOM_MATERIAL_DATA_SPACE);

MeshPBRResources CreateMeshPBRResources()
{
    MeshPBRResources resources;
    resources.Transform = MeshPBRResources_Transform;
    resources.BoneTransformOffset = MeshPBRResources_BoneTransformOffset;
    resources.AlbedoColor = MeshPBRResources_AlbedoColor;
    resources.Metalness = MeshPBRResources_Metalness;
    resources.Roughness = MeshPBRResources_Roughness;
    resources.UseAlbedoMap = MeshPBRResources_UseAlbedoMap;
    resources.UseNormalMap = MeshPBRResources_UseNormalMap;
    resources.UseMetalnessMap = MeshPBRResources_UseMetalnessMap;
    resources.UseRoughnessMap = MeshPBRResources_UseRoughnessMap;
    resources.AlbedoMap = MeshPBRResources_AlbedoMap;
    resources.NormalMap = MeshPBRResources_NormalMap;
    resources.MetalnessMap = MeshPBRResources_MetalnessMap;
    resources.RoughnessMap = MeshPBRResources_RoughnessMap;
    resources.AlbedoMapSampler = MeshPBRResources_AlbedoMapSampler;
    resources.NormalMapSampler = MeshPBRResources_NormalMapSampler;
    resources.MetalnessMapSampler = MeshPBRResources_MetalnessMapSampler;
    resources.RoughnessMapSampler = MeshPBRResources_RoughnessMapSampler;
    return resources;
}

#endif // MESHPBRRESOURCES_HLSLI