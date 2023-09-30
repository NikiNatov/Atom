#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Renderer/CommandBuffer.h"
#include "Atom/Renderer/RenderGraph/ResourceView.h"
#include "Atom/Renderer/RenderGraph/RenderPass.h"

namespace Atom
{
    class ResourceScheduler;
    class SceneFrameData;

    class RenderPassContext
    {
    public:
        RenderPassContext(RenderPassID passID, Ref<CommandBuffer> cmdBuffer, const ResourceScheduler& resourceScheduler, const SceneFrameData& sceneData);

        ResourceView<TextureUAV>* GetUA(ResourceID_UA id) const;
        ResourceView<TextureSRV>* GetSR(ResourceID_UA id) const;

        ResourceView<SurfaceRTV>* GetRT(ResourceID_RT id) const;
        ResourceView<SurfaceSRV>* GetSR(ResourceID_RT id) const;

        ResourceView<SurfaceDSV_RW>* GetDS_RW(ResourceID_DS id) const;
        ResourceView<SurfaceDSV_RO>* GetDS_RO(ResourceID_DS id) const;
        ResourceView<SurfaceSRV>* GetSR(ResourceID_DS id) const;

        Ref<ConstantBuffer> GetFrameConstantBuffer() const;
        const DescriptorAllocation& GetFrameResourceTable() const;
        const DescriptorAllocation& GetFrameSamplerTable() const;

        inline RenderPassID GetPassID() const { return m_PassID; }
        inline Ref<CommandBuffer> GetCommandBuffer() const { return m_CommandBuffer; }
        inline const SceneFrameData& GetSceneData() const { return m_SceneData; }

    private:
        IResourceView* FindInput(ResourceID id) const;
        IResourceView* FindOutput(ResourceID id) const;

    private:
        RenderPassID             m_PassID;
        Ref<CommandBuffer>       m_CommandBuffer;
        const ResourceScheduler& m_ResourceScheduler;
        const SceneFrameData&    m_SceneData;
    };
}