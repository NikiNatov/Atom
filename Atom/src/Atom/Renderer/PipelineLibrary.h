#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Core/Hash.h"
#include "Atom/Renderer/Pipeline.h"

namespace Atom
{
    class PipelineLibrary
    {
    public:
        PipelineLibrary();

        Ref<GraphicsPipeline> LoadGraphicsPipeline(const GraphicsPipelineDescription& description, const char* debugName = "Unnamed Gfx Pipeline");
        Ref<ComputePipeline> LoadComputePipeline(const ComputePipelineDescription& description, const char* debugName = "Unnamed Compute Pipeline");

        Ref<GraphicsPipeline> GetGraphicsPipeline(const GraphicsPipelineDescription& description) const;
        Ref<ComputePipeline> GetComputePipeline(const ComputePipelineDescription& description) const;

        void Clear();
    public:
        inline static PipelineLibrary& Get() { return *ms_Instance; }
    private:
        HashMap<u64, Ref<GraphicsPipeline>> m_GfxPipelines;
        HashMap<u64, Ref<ComputePipeline>>  m_ComputePipelines;
    private:
        inline static PipelineLibrary* ms_Instance = nullptr;
    };
}