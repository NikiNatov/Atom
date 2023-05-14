#ifndef __COMMON_HLSLI__
#define __COMMON_HLSLI__

#define ATOM_INSTANCE_DATA_SPACE space0
#define ATOM_MATERIAL_DATA_SPACE space1
#define ATOM_PASS_DATA_SPACE space2
#define ATOM_FRAME_DATA_SPACE space3

static const float PI = 3.141592;
static const float TwoPI = 2 * PI;
static const float Epsilon = 0.00001;

struct Light
{
    float4 Position;
    float4 Direction;
    float4 Color;
    float  Intensity;
    float  ConeAngle;
    float3 AttenuationFactors;
    uint   LightType;
};

#endif // __COMMON_HLSLI__