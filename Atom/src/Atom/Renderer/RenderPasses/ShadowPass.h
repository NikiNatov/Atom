#pragma once

#include "Atom/Core/Core.h"

#include "Atom/Renderer/RenderGraph/RenderPass.h"
#include "Atom/Renderer/RenderGraph/RenderPassBuilder.h"
#include "Atom/Renderer/RenderGraph/RenderPassContext.h"

#include "Atom/Renderer/Renderer.h"

namespace Atom
{
    class ShadowPass : public RenderPass
    {
    public:
        ShadowPass(RenderPassID passID, const String& name, const Vector<ShadowCascade>& shadowCascades, const Vector<MeshEntry>& meshEntries, bool isAnimated);

        virtual void Build(RenderPassBuilder& builder) override;
        virtual void Execute(RenderPassContext& context) override;

    private:
        const Vector<ShadowCascade>& m_ShadowCascades;
        const Vector<MeshEntry>& m_MeshEntries;
        bool m_IsAnimated;
    };
}