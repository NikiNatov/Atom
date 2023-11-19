#pragma once

#include "Atom/Core/Core.h"

#include "Atom/Renderer/RenderGraph/RenderPass.h"
#include "Atom/Renderer/RenderGraph/RenderPassBuilder.h"
#include "Atom/Renderer/RenderGraph/RenderPassContext.h"

#include "Atom/Renderer/Renderer.h"

namespace Atom
{
    class GeometryPass : public RenderPass
    {
    public:
        GeometryPass(RenderPassID passID, const String& name, const Vector<MeshEntry>& meshEntries, bool isAnimated);

        virtual void Build(RenderPassBuilder& builder) override;
        virtual void Execute(RenderPassContext& context) override;

    private:
        const Vector<MeshEntry>& m_MeshEntries;
        bool m_IsAnimated;
    };
}