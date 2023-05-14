#ifndef __BRDFRESOURCES_HLSLI__
#define __BRDFRESOURCES_HLSLI__

#include "include/Common.hlsli"

struct BRDFResources
{
    RWTexture2D<float2> BRDFTexture;
};

// -------------------------------------------------- Resources --------------------------------------------------- //
RWTexture2D<float2> BRDFResources_BRDFTexture : register(u0, ATOM_INSTANCE_DATA_SPACE);

BRDFResources CreateBRDFResources()
{
    BRDFResources resources;
    resources.BRDFTexture = BRDFResources_BRDFTexture;
    return resources;
}

#endif // __BRDFRESOURCES_HLSLI__