struct Light
{
    float4 Position;
    float4 Direction;
    float4 Color;
    float  Intensity;
    float  ConeAngle;
    float3 AttenuationFactors;
    uint   LightType;
}

ShaderInputGroup FrameParams<BindTo=DefaultLayout::Frame>
{
    matrix                   ViewMatrix;
    matrix                   ProjectionMatrix;
    matrix                   InvViewProjMatrix;
    float3                   CameraPosition;
    float                    CameraExposure;
    uint                     NumLights;

    TextureCube<float4>      EnvironmentMap;
    TextureCube<float4>      IrradianceMap;
    Texture2D<float4>        BRDFMap;
    StructuredBuffer<Light>  Lights;
    StructuredBuffer<matrix> BoneTransforms;
    SamplerState             EnvironmentMapSampler;
    SamplerState             IrradianceMapSampler;
    SamplerState             BRDFMapSampler;
}