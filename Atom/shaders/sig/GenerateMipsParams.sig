ShaderInputGroup GenerateMipsParams<BindTo=DefaultLayout::Instance>
{
    float2              TexelSize;
    uint                TopMipLevel;

    Texture2D<float4>   SrcTexture;
    RWTexture2D<float4> DstTexture;
}