ShaderInputGroup<BindPoint::Instance> ImGuiParams
{
    matrix            MVPMatrix;

    Texture2D<float4> Texture;
    SamplerState      TextureSampler;
}