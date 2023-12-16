ShaderInputLayout DefaultLayout
{
    BindPoint Instance;
    BindPoint Material;
    BindPoint Pass;
    BindPoint Frame;

    StaticSampler LinearClampSampler = { SamplerFilter::Linear, SamplerWrap::Clamp };
    StaticSampler LinearRepeatSampler = { SamplerFilter::Linear, SamplerWrap::Repeat };
    StaticSampler NearestClampSampler = { SamplerFilter::Nearest, SamplerWrap::Clamp };
    StaticSampler NearestRepeatSampler = { SamplerFilter::Nearest, SamplerWrap::Repeat };
    StaticSampler AnisotropicClampSampler = { SamplerFilter::Anisotropic, SamplerWrap::Clamp };
    StaticSampler AnisotropicRepeatSampler = { SamplerFilter::Anisotropic, SamplerWrap::Repeat };
}