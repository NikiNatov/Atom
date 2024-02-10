ShaderInputGroup BloomUpsampleParams<BindTo=DefaultLayout::Instance>
{
    float FilterRadius;
    float SourceMipLevel;

    Texture2D<float4> SourceTexture;
    RWTexture2D<float4> UpsampledTexture;
}