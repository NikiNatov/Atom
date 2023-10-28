ShaderInputGroup<BindPoint::Instance> CubeMapPrefilterParams
{
    float                    Roughness;

    TextureCube<float4>      EnvMapUnfiltered;
    RWTexture2DArray<float4> EnvMap;
    SamplerState             EnvMapSampler;
}