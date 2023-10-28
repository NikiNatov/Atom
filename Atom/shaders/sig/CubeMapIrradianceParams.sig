ShaderInputGroup<BindPoint::Instance> CubeMapIrradianceParams
{
    TextureCube<float4>      EnvMap;
    RWTexture2DArray<float4> IrradianceMap;
    SamplerState             EnvMapSampler;
}