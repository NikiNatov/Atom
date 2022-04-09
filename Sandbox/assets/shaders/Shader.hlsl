struct Mat
{
    matrix ModelMatrix;
    matrix ModelViewMatrix;
    matrix InverseTransposeModelViewMatrix;
    matrix ModelViewProjectionMatrix;
};

ConstantBuffer<Mat> MatCB : register(b0);

struct VertexPositionNormalTexture
{
    float3 Position : POSITION;
    float3 Normal   : NORMAL;
    float2 TexCoord : TEXCOORD;
};

struct VertexShaderOutput
{
    float4 PositionVS : POSITION;
    float3 NormalVS   : NORMAL;
    float2 TexCoord   : TEXCOORD;
    float4 Position   : SV_Position;
};

VertexShaderOutput VSMain(VertexPositionNormalTexture IN)
{
    VertexShaderOutput OUT;

    OUT.Position = mul(MatCB.ModelViewProjectionMatrix, float4(IN.Position, 1.0f));
    OUT.PositionVS = mul(MatCB.ModelViewMatrix, float4(IN.Position, 1.0f));
    OUT.NormalVS = mul((float3x3)MatCB.InverseTransposeModelViewMatrix, IN.Normal);
    OUT.TexCoord = IN.TexCoord;

    return OUT;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
struct PixelShaderInput
{
    float4 PositionVS : POSITION;
    float3 NormalVS   : NORMAL;
    float2 TexCoord   : TEXCOORD;
};

struct Material
{
    float4 Diffuse;
    //------------------------------------ ( 16 bytes )
    float4 Specular;
    //------------------------------------ ( 16 bytes )
    float4 Emissive;
    //------------------------------------ ( 16 bytes )
    float4 Ambient;
    //------------------------------------ ( 16 bytes )
    float4 Reflectance;
    //------------------------------------ ( 16 bytes )
    float Opacity;  // If Opacity < 1, then the material is transparent.
    float SpecularPower;
    float IndexOfRefraction;  // For transparent materials, IOR > 0.
    float BumpIntensity;      // When using bump textures (height maps) we need
                              // to scale the height values so the normals are visible.
    //------------------------------------ ( 16 bytes )
    float AlphaThreshold;  // Pixels with alpha < m_AlphaThreshold will be discarded.
    bool  HasAmbientTexture;
    bool  HasEmissiveTexture;
    bool  HasDiffuseTexture;
    //------------------------------------ ( 16 bytes )
    bool HasSpecularTexture;
    bool HasSpecularPowerTexture;
    bool HasNormalTexture;
    bool HasBumpTexture;
    //------------------------------------ ( 16 bytes )
    bool   HasOpacityTexture;
    float3 Padding;  // Pad to 16 byte boundary.
    //------------------------------------ ( 16 bytes )
    // Total:                              ( 16 * 9 = 144 bytes )
};

struct PointLight
{
    float4 PositionWS; // Light position in world space.
    //----------------------------------- (16 byte boundary)
    float4 PositionVS; // Light position in view space.
    //----------------------------------- (16 byte boundary)
    float4 Color;
    //----------------------------------- (16 byte boundary)
    float       Intensity;
    float       Attenuation;
    float2      Padding;                // Pad to 16 bytes
    //----------------------------------- (16 byte boundary)
    // Total:                              16 * 4 = 64 bytes
};

struct SpotLight
{
    float4 PositionWS; // Light position in world space.
    //----------------------------------- (16 byte boundary)
    float4 PositionVS; // Light position in view space.
    //----------------------------------- (16 byte boundary)
    float4 DirectionWS; // Light direction in world space.
    //----------------------------------- (16 byte boundary)
    float4 DirectionVS; // Light direction in view space.
    //----------------------------------- (16 byte boundary)
    float4 Color;
    //----------------------------------- (16 byte boundary)
    float       Intensity;
    float       SpotAngle;
    float       Attenuation;
    float       Padding;                // Pad to 16 bytes.
    //----------------------------------- (16 byte boundary)
    // Total:                              16 * 6 = 96 bytes
};

struct LightProperties
{
    uint NumPointLights;
    uint NumSpotLights;
};

struct LightResult
{
    float4 Diffuse;
    float4 Specular;
};

ConstantBuffer<Material> mat_MaterialCB : register(b0, space1);
ConstantBuffer<LightProperties> LightPropertiesCB : register(b1);

StructuredBuffer<PointLight> PointLights : register(t0);
StructuredBuffer<SpotLight> SpotLights : register(t1);
Texture2D DiffuseTexture            : register(t2);

SamplerState LinearRepeatSampler    : register(s0);

float3 LinearToSRGB(float3 x)
{
    // This is exactly the sRGB curve
    //return x < 0.0031308 ? 12.92 * x : 1.055 * pow(abs(x), 1.0 / 2.4) - 0.055;

    // This is cheaper but nearly equivalent
    return x < 0.0031308 ? 12.92 * x : 1.13005 * sqrt(abs(x - 0.00228)) - 0.13448 * x + 0.005719;
}

float DoDiffuse(float3 N, float3 L)
{
    return max(0, dot(N, L));
}

float DoSpecular(float3 V, float3 N, float3 L)
{
    float3 R = normalize(reflect(-L, N));
    float RdotV = max(0, dot(R, V));

    return pow(RdotV, mat_MaterialCB.SpecularPower);
}

float DoAttenuation(float attenuation, float distance)
{
    return 1.0f / (1.0f + attenuation * distance * distance);
}

float DoSpotCone(float3 spotDir, float3 L, float spotAngle)
{
    float minCos = cos(spotAngle);
    float maxCos = (minCos + 1.0f) / 2.0f;
    float cosAngle = dot(spotDir, -L);
    return smoothstep(minCos, maxCos, cosAngle);
}

LightResult DoPointLight(PointLight light, float3 V, float3 P, float3 N)
{
    LightResult result;
    float3 L = (light.PositionVS.xyz - P);
    float d = length(L);
    L = L / d;

    float attenuation = DoAttenuation(light.Attenuation, d);

    result.Diffuse = DoDiffuse(N, L) * attenuation * light.Color * light.Intensity;
    result.Specular = DoSpecular(V, N, L) * attenuation * light.Color * light.Intensity;

    return result;
}

LightResult DoSpotLight(SpotLight light, float3 V, float3 P, float3 N)
{
    LightResult result;
    float3 L = (light.PositionVS.xyz - P);
    float d = length(L);
    L = L / d;

    float attenuation = DoAttenuation(light.Attenuation, d);

    float spotIntensity = DoSpotCone(light.DirectionVS.xyz, L, light.SpotAngle);

    result.Diffuse = DoDiffuse(N, L) * attenuation * spotIntensity * light.Color * light.Intensity;
    result.Specular = DoSpecular(V, N, L) * attenuation * spotIntensity * light.Color * light.Intensity;

    return result;
}

LightResult DoLighting(float3 P, float3 N)
{
    uint i;

    // Lighting is performed in view space.
    float3 V = normalize(-P);

    LightResult totalResult = (LightResult)0;

    for (i = 0; i < LightPropertiesCB.NumPointLights; ++i)
    {
        LightResult result = DoPointLight(PointLights[i], V, P, N);

        totalResult.Diffuse += result.Diffuse;
        totalResult.Specular += result.Specular;
    }

    for (i = 0; i < LightPropertiesCB.NumSpotLights; ++i)
    {
        LightResult result = DoSpotLight(SpotLights[i], V, P, N);

        totalResult.Diffuse += result.Diffuse;
        totalResult.Specular += result.Specular;
    }

    return totalResult;
}

float4 PSMain(PixelShaderInput IN) : SV_Target
{
    LightResult lit = DoLighting(IN.PositionVS.xyz, normalize(IN.NormalVS));

    tex[float2(0.0, 0.0)] = float2(1.0, 0.0);

    float4 emissive = mat_MaterialCB.Emissive;
    float4 ambient = mat_MaterialCB.Ambient;
    float4 diffuse = mat_MaterialCB.Diffuse * lit.Diffuse;
    float4 specular = mat_MaterialCB.Specular * lit.Specular;
    float4 texColor = DiffuseTexture.Sample(LinearRepeatSampler, IN.TexCoord);

    return (emissive + ambient + diffuse + specular) * texColor;
}