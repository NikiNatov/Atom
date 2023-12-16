ShaderInputGroup CubeMapPrefilterParams<BindTo=DefaultLayout::Instance>
{
    float                    Roughness;

    TextureCube<float4>      EnvMapUnfiltered;
    RWTexture2DArray<float4> EnvMap;
}