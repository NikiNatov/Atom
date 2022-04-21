#pragma once

#include "Atom/Renderer/API/GraphicsPipeline.h"

#if defined(ATOM_PLATFORM_WINDOWS)

#include "DirectX12.h"

namespace Atom
{
    class DX12GraphicsPipeline : public GraphicsPipeline
    {
    public:
        DX12GraphicsPipeline(const GraphicsPipelineDescription& description, const char* debugName = "Unnamed Graphics Pipeline");
        ~DX12GraphicsPipeline();

        virtual const PipelineLayout& GetLayout() const override;
        virtual const Shader* GetShader() const override;
        virtual const Framebuffer* GetFramebuffer() const override;
        virtual Topology GetTopology() const override;
        virtual bool IsDepthTestingEnabled() const override;
        virtual bool IsBlendingEnabled() const override;
        virtual bool IsWireframe() const override;
        virtual bool IsBackfaceCullingEnabled() const override;

        inline ComPtr<ID3D12PipelineState> GetD3DPipeline() const { return m_D3DPipeline; }
        inline const D3D12_GRAPHICS_PIPELINE_STATE_DESC& GetD3DDescription() const { return m_D3DDescription; }
    private:
        GraphicsPipelineDescription        m_Description;
        D3D12_GRAPHICS_PIPELINE_STATE_DESC m_D3DDescription {};
        ComPtr<ID3D12PipelineState>        m_D3DPipeline;
    };

}

#endif // ATOM_PLATFORM_WINDOWS