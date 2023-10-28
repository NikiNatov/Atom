ShaderInputGroup<BindPoint::Instance> EquirectToCubeMapParams
{
    uint                     MipLevel;

    Texture2D<float4>        InputTexture;
    RWTexture2DArray<float4> OutputTexture;
    SamplerState             InputTextureSampler;
}