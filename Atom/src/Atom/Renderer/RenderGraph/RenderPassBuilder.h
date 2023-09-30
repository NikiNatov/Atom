#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Renderer/RenderGraph/ResourceID.h"
#include "Atom/Renderer/RenderGraph/RenderPass.h"
#include "Atom/Renderer/RenderGraph/TextureResource.h"
#include "Atom/Renderer/RenderGraph/RenderSurfaceResource.h"

namespace Atom
{
    class ResourceScheduler;

    class RenderPassBuilder
    {
    public:
        RenderPassBuilder(RenderPassID passID, ResourceScheduler& resourceScheduler);

        void NewUA(ResourceID_UA id, const ResourceID_UA::ResourceType::ResourceDescType& description);
        void NewUA(ResourceID_UA id, ResourceID_UA::ResourceType::HWResourceType* externalResource);
        void NewRT(ResourceID_RT id, const ResourceID_RT::ResourceType::ResourceDescType& description);
        void NewRT(ResourceID_RT id, ResourceID_RT::ResourceType::HWResourceType* externalResource);
        void NewDS(ResourceID_DS id, const ResourceID_DS::ResourceType::ResourceDescType& description);
        void NewDS(ResourceID_DS id, ResourceID_DS::ResourceType::HWResourceType* externalResource);

        void Read(ResourceID_UA id);
        void Read(ResourceID_RT id);
        void Read(ResourceID_DS id, bool isSRV = false);

        void Write(ResourceID_UA id);
        void Write(ResourceID_RT id);
        void Write(ResourceID_DS id);

        inline RenderPassID GetPassID() const { return m_PassID; }
    private:
        ResourceScheduler& m_ResourceScheduler;
        RenderPassID       m_PassID;
    };
}