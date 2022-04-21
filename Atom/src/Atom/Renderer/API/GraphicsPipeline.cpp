#include "atompch.h"
#include "GraphicsPipeline.h"

#include "Atom/Platform/DirectX12/DX12GraphicsPipeline.h"

namespace Atom
{
    Ref<GraphicsPipeline> GraphicsPipeline::Create(const GraphicsPipelineDescription& description, const char* debugName)
    {
        switch (Renderer::GetAPI())
        {

#if defined(ATOM_PLATFORM_WINDOWS)
            case RenderAPI::DirectX12: return CreateRef<DX12GraphicsPipeline>(description, debugName);
#endif // ATOM_PLATFORM_WINDOWS

        }

        ATOM_ASSERT(false, "Unknown API!");
        return nullptr;
    }
}
