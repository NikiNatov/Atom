#pragma once

#include "Atom/Core/Core.h"

namespace Atom
{
    enum class ResourceState : u32
    {
        Common                          = 0,
        VertexConstantBuffer            = BIT(0),
        IndexBuffer                     = BIT(1),
        RenderTarget                    = BIT(2),
        UnorderedAccess                 = BIT(3),
        DepthWrite                      = BIT(4),
        DepthRead                       = BIT(5),
        NonPixelShaderRead              = BIT(6),
        PixelShaderRead                 = BIT(7),
        StreamOut                       = BIT(8),
        IndirectArgument                = BIT(9),
        CopyDestination                 = BIT(10),
        CopySource                      = BIT(11),
        ResolveDestination              = BIT(12),
        ResolveSource                   = BIT(13),
        RaytracingAccelerationStructure = BIT(14),

        Present = Common,
        GenericRead = VertexConstantBuffer | IndexBuffer | NonPixelShaderRead | PixelShaderRead | IndirectArgument | CopySource,
        AnyShaderRead = PixelShaderRead | NonPixelShaderRead,
        Predication = IndirectArgument
    };

    IMPL_ENUM_OPERATORS(ResourceState)
}