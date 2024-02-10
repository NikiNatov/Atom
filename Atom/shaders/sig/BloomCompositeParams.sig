ShaderInputGroup BloomCompositeParams<BindTo=DefaultLayout::Pass>
{
    float BloomStrength;

    Texture2D<float4> SceneTexture;
    Texture2D<float4> BloomTexture;
}