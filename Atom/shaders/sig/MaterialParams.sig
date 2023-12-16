ShaderInputGroup MaterialParams<BindTo=DefaultLayout::Material>
{
    float4            AlbedoColor;
    float             Metalness;
    float             Roughness;
    int               UseAlbedoMap;
    int               UseNormalMap;
    int               UseMetalnessMap;
    int               UseRoughnessMap;

    Texture2D<float4> AlbedoMap;
    Texture2D<float4> NormalMap;
    Texture2D<float4> MetalnessMap;
    Texture2D<float4> RoughnessMap;
    SamplerState      AlbedoMapSampler;
    SamplerState      NormalMapSampler;
    SamplerState      MetalnessMapSampler;
    SamplerState      RoughnessMapSampler;
}