#pragma once

#include "Atom/Core/Core.h"

#include "Atom/Renderer/RenderGraph/RenderPass.h"
#include "Atom/Renderer/RenderGraph/RenderPassBuilder.h"
#include "Atom/Renderer/RenderGraph/RenderPassContext.h"

#include "Atom/Renderer/Renderer.h"

namespace Atom
{
    class BloomDownsamplePass : public RenderPass
    {
    public:
        BloomDownsamplePass(RenderPassID passID, const String& name, u32 viewportWidth, u32 viewportHeight, const BloomSettings& settings);

        virtual void Build(RenderPassBuilder& builder) override;
        virtual void Execute(RenderPassContext& context) override;
    private:
        u32 m_ViewportWidth;
        u32 m_ViewportHeight;
        BloomSettings m_Settings;
    };

    class BloomUpsamplePass : public RenderPass
    {
    public:
        BloomUpsamplePass(RenderPassID passID, const String& name, u32 viewportWidth, u32 viewportHeight, const BloomSettings& settings);

        virtual void Build(RenderPassBuilder& builder) override;
        virtual void Execute(RenderPassContext& context) override;
    private:
        u32 m_ViewportWidth;
        u32 m_ViewportHeight;
        BloomSettings m_Settings;
    };

    class BloomCompositePass : public RenderPass
    {
    public:
        BloomCompositePass(RenderPassID passID, const String& name, u32 viewportWidth, u32 viewportHeight, const BloomSettings& settings);

        virtual void Build(RenderPassBuilder& builder) override;
        virtual void Execute(RenderPassContext& context) override;
    private:
        u32 m_ViewportWidth;
        u32 m_ViewportHeight;
        BloomSettings m_Settings;
    };
}