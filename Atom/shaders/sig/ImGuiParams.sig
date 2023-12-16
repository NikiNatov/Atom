ShaderInputGroup ImGuiPassParams<BindTo=DefaultLayout::Pass>
{
    matrix MVPMatrix;
}

ShaderInputGroup ImGuiTextureParams<BindTo=DefaultLayout::Instance>
{
    Texture2D<float4> Texture;
}