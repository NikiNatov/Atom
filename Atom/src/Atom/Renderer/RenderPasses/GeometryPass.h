#pragma once

#include "Atom/Core/Core.h"

#include "Atom/Renderer/RenderGraph/RenderPass.h"
#include "Atom/Renderer/RenderGraph/RenderPassBuilder.h"
#include "Atom/Renderer/RenderGraph/RenderPassContext.h"

namespace Atom
{
    class GeometryPass : public RenderPass
    {
    public:
        GeometryPass(RenderPassID passID, const String& name);

        virtual void Build(RenderPassBuilder& builder) override;
        virtual void Execute(RenderPassContext& context) override;

    };
}