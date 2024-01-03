ShaderInputGroup ShadowDepthPassParams<BindTo=DefaultLayout::Pass>
{
    uint CascadeIndex;
}

ShaderInputGroup ShadowParams<BindTo=DefaultLayout::Pass>
{
    float DepthBias;
    Texture2DArray<float> CascadeShadowMap;
}