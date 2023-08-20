#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Renderer/Texture.h"
#include "Atom/Renderer/RenderGraph/ResourceID.h"
#include "Atom/Renderer/RenderGraph/RenderPass.h"
#include "Atom/Renderer/RenderGraph/RenderPassContext.h"

namespace Atom
{
    class RenderGraph;
    class IResourceView;

    class RenderPassBuilder
    {
    public:
        RenderPassBuilder(RenderGraph& graph, RenderPassID passID);

        RenderPassContext* Finalize();

        void NewUA(ResourceID_UA id, const TextureDescription& description);
        void NewRT(ResourceID_RT id, const TextureDescription& description);
        void NewDS(ResourceID_DS id, const TextureDescription& description);

        void Read(ResourceID_UA id);
        void Read(ResourceID_RT id);
        void Read(ResourceID_DS id, bool isSRV = false);

        void Write(ResourceID_UA id);
        void Write(ResourceID_RT id);
        void Write(ResourceID_DS id);

        inline RenderPassID GetPassID() const { return m_PassID; }
    private:
        bool HasInput(ResourceID id) const;
        bool HasOutput(ResourceID id) const;
    private:
        RenderGraph&                 m_Graph;
        RenderPassID                 m_PassID;
        Vector<Scope<IResourceView>> m_Inputs;
        Vector<Scope<IResourceView>> m_Outputs;
    };
}