#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Renderer/CommandBuffer.h"
#include "Atom/Renderer/RenderGraph/ResourceView.h"
#include "Atom/Renderer/RenderGraph/RenderPass.h"

namespace Atom
{
    class RenderPassContext
    {
    public:
        RenderPassContext(const RenderPass& pass, Vector<Scope<IResourceView>>&& inputs, Vector<Scope<IResourceView>>&& outputs);

        const ResourceView<TextureUAV>& GetUA(ResourceID_UA id) const;
        const ResourceView<TextureSRV>& GetSR(ResourceID_UA id) const;

        const ResourceView<SurfaceRTV>& GetRT(ResourceID_RT id) const;
        const ResourceView<TextureSRV>& GetSR(ResourceID_RT id) const;

        const ResourceView<SurfaceDSV>& GetDS(ResourceID_DS id) const;
        const ResourceView<TextureSRV>& GetSR(ResourceID_DS id) const;

        inline Ref<CommandBuffer> GetCommandBuffer() const { return m_CommandBuffer; }
        inline const Vector<Scope<IResourceView>>& GetInputs() const { return m_Inputs; }
        inline const Vector<Scope<IResourceView>>& GetOutputs() const { return m_Outputs; }
    private:
        const IResourceView* FindInput(ResourceID id) const;
        const IResourceView* FindOutput(ResourceID id) const;
    private:
        Ref<CommandBuffer>           m_CommandBuffer;
        Vector<Scope<IResourceView>> m_Inputs;
        Vector<Scope<IResourceView>> m_Outputs;
    };
}