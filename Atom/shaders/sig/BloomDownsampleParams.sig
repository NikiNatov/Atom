ShaderInputGroup BloomDownsampleParams<BindTo=DefaultLayout::Instance>
{
    float SourceMipLevel;
    float DownsampledMipLevel;

    Texture2D<float4> SourceTexture;
    RWTexture2D<float4> DownsampledTexture;
}