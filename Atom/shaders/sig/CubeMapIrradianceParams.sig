ShaderInputGroup CubeMapIrradianceParams<BindTo=DefaultLayout::Instance>
{
    TextureCube<float4>      EnvMap;
    RWTexture2DArray<float4> IrradianceMap;
}