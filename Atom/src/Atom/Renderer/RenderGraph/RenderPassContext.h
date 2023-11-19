#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Renderer/CommandBuffer.h"
#include "Atom/Renderer/RenderGraph/ResourceView.h"
#include "Atom/Renderer/RenderGraph/RenderPass.h"

namespace Atom
{
    class ResourceScheduler;

    class RenderPassContext
    {
    public:
        RenderPassContext(RenderPassID passID, Ref<CommandBuffer> cmdBuffer, const ResourceScheduler& resourceScheduler);

        ResourceView<TextureUAV>* GetUA(const ResourceID_UA& id) const;
        ResourceView<TextureSRV>* GetSR(const ResourceID_UA& id) const;

        ResourceView<SurfaceRTV>* GetRT(const ResourceID_RT& id) const;
        ResourceView<SurfaceSRV>* GetSR(const ResourceID_RT& id) const;

        ResourceView<SurfaceDSV_RW>* GetDS_RW(const ResourceID_DS& id) const;
        ResourceView<SurfaceDSV_RO>* GetDS_RO(const ResourceID_DS& id) const;
        ResourceView<SurfaceSRV>* GetSR(const ResourceID_DS& id) const;

        inline RenderPassID GetPassID() const { return m_PassID; }
        inline Ref<CommandBuffer> GetCommandBuffer() const { return m_CommandBuffer; }

    private:
        IResourceView* FindInput(const ResourceID& id) const;
        IResourceView* FindOutput(const ResourceID& id) const;

    private:
        RenderPassID             m_PassID;
        Ref<CommandBuffer>       m_CommandBuffer;
        const ResourceScheduler& m_ResourceScheduler;
    };
}