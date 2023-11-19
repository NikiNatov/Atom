#include "atompch.h"
#include "PipelineLibrary.h"

namespace Atom
{
    struct GraphicsPipelineHash
    {
        u64 operator()(const GraphicsPipelineDescription& desc) const
        {
            HashBuilder hashBuilder;
            hashBuilder.AddToHash(desc.Shader->GetHash());

            for (const GraphicsPipelineLayout::Element& element : desc.Layout)
                hashBuilder.AddToHash(element.Type);

            for (TextureFormat format : desc.RenderTargetFormats)
                hashBuilder.AddToHash(format);

            hashBuilder.AddToHash(desc.Topology);
            hashBuilder.AddToHash(desc.EnableDepthTest);
            hashBuilder.AddToHash(desc.EnableBlend);
            hashBuilder.AddToHash(desc.Wireframe);
            hashBuilder.AddToHash(desc.BackfaceCulling);
            return hashBuilder.GetHash();
        }
    };

    struct ComputePipelineHash
    {
        u64 operator()(const ComputePipelineDescription& desc) const
        {
            return desc.Shader->GetHash();
        }
    };

    // -----------------------------------------------------------------------------------------------------------------------------
    PipelineLibrary::PipelineLibrary()
    {
        ATOM_ENGINE_ASSERT(ms_Instance == nullptr, "Pipeline library already exists!");
        ms_Instance = this;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<GraphicsPipeline> PipelineLibrary::LoadGraphicsPipeline(const GraphicsPipelineDescription& description, const char* debugName)
    {
        if (Ref<GraphicsPipeline> existingPipeline = GetGraphicsPipeline(description))
            return existingPipeline;

        u64 hash = GraphicsPipelineHash{}(description);
        Ref<GraphicsPipeline> pipeline = CreateRef<GraphicsPipeline>(description, debugName);
        m_GfxPipelines[hash] = pipeline;
        return pipeline;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<ComputePipeline> PipelineLibrary::LoadComputePipeline(const ComputePipelineDescription& description, const char* debugName)
    {
        if (Ref<ComputePipeline> existingPipeline = GetComputePipeline(description))
            return existingPipeline;

        u64 hash = ComputePipelineHash{}(description);
        Ref<ComputePipeline> pipeline = CreateRef<ComputePipeline>(description, debugName);
        m_ComputePipelines[hash] = pipeline;
        return pipeline;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<GraphicsPipeline> PipelineLibrary::GetGraphicsPipeline(const GraphicsPipelineDescription& description) const
    {
        u64 hash = GraphicsPipelineHash{}(description);
        auto it = m_GfxPipelines.find(hash);

        if (it == m_GfxPipelines.end())
            return nullptr;

        return it->second;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<ComputePipeline> PipelineLibrary::GetComputePipeline(const ComputePipelineDescription& description) const
    {
        u64 hash = ComputePipelineHash{}(description);
        auto it = m_ComputePipelines.find(hash);

        if (it == m_ComputePipelines.end())
            return nullptr;

        return it->second;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void PipelineLibrary::Clear()
    {
        m_GfxPipelines.clear();
        m_ComputePipelines.clear();
    }
}
