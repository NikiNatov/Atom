#pragma once

#include "Atom/Core/Core.h"

#include "Atom/Renderer/RenderGraph/RenderPass.h"
#include "Atom/Renderer/RenderGraph/RenderPassBuilder.h"
#include "Atom/Renderer/RenderGraph/RenderPassContext.h"

namespace Atom
{
    class CompositePass : public RenderPass
    {
    public:
        CompositePass(RenderPassID passID, const String& name, u32 viewportWidth, u32 viewportHeight, bool renderToSwapChain);

        virtual void Build(RenderPassBuilder& builder) override;
        virtual void Execute(RenderPassContext& context) override;
    private:
        u32 m_ViewportWidth;
        u32 m_ViewportHeight;
        bool m_RenderToSwapChain;
    };
}