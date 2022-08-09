#include "atompch.h"

#include "Atom/Core/DirectX12/DirectX12Utils.h"

#include "GraphicsPipeline.h"
#include "Device.h"
#include "Framebuffer.h"

namespace Atom
{
    namespace Utils
    {
        DXGI_FORMAT ShaderDataTypeToDXGIFormat(ShaderDataType type)
        {
            switch (type)
            {
                case ShaderDataType::Float:		return DXGI_FORMAT_R32_FLOAT;
                case ShaderDataType::Float2:	return DXGI_FORMAT_R32G32_FLOAT;
                case ShaderDataType::Float3:	return DXGI_FORMAT_R32G32B32_FLOAT;
                case ShaderDataType::Float4:	return DXGI_FORMAT_R32G32B32A32_FLOAT;
                case ShaderDataType::Int:		return DXGI_FORMAT_R32_SINT;
                case ShaderDataType::Int2:		return DXGI_FORMAT_R32G32_SINT;
                case ShaderDataType::Int3:		return DXGI_FORMAT_R32G32B32_SINT;
                case ShaderDataType::Int4:		return DXGI_FORMAT_R32G32B32A32_SINT;
            }

            ATOM_ENGINE_ASSERT(false, "Unknown data type!");
            return DXGI_FORMAT_UNKNOWN;
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    GraphicsPipeline::GraphicsPipeline(const GraphicsPipelineDescription& description, const char* debugName)
        : m_Description(description)
    {
        //////////////////////////////////////////////////////////////////////
        ///                            Input layout                         //
        //////////////////////////////////////////////////////////////////////
        auto& elements = m_Description.Layout.GetElements();
        Vector<D3D12_INPUT_ELEMENT_DESC> inputLayoutElements(elements.size());

        for (u32 i = 0; i < elements.size(); i++)
        {
            inputLayoutElements[i] = { elements[i].SemanticName.c_str(),
                                       0,
                                       Utils::ShaderDataTypeToDXGIFormat(elements[i].Type),
                                       0,
                                       elements[i].Offset,
                                       D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
                                       0 };
        }

        D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = {};
        inputLayoutDesc.NumElements = inputLayoutElements.size();
        inputLayoutDesc.pInputElementDescs = inputLayoutElements.data();

        //////////////////////////////////////////////////////////////////////
        ///                             States                              //
        //////////////////////////////////////////////////////////////////////
        CD3DX12_RASTERIZER_DESC rasterizerState = {};
        rasterizerState.CullMode = m_Description.BackfaceCulling ? D3D12_CULL_MODE_BACK : D3D12_CULL_MODE_NONE;
        rasterizerState.FillMode = m_Description.Wireframe ? D3D12_FILL_MODE_WIREFRAME : D3D12_FILL_MODE_SOLID;
        rasterizerState.AntialiasedLineEnable = false;
        rasterizerState.FrontCounterClockwise = true;
        rasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
        rasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
        rasterizerState.DepthClipEnable = true;
        rasterizerState.MultisampleEnable = false;
        rasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;

        CD3DX12_BLEND_DESC blendState = {};

        if (m_Description.EnableBlend)
        {
            blendState.AlphaToCoverageEnable = false;
            blendState.IndependentBlendEnable = false;
            blendState.RenderTarget[0].BlendEnable = true;
            blendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
            blendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
            blendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
            blendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_SRC_ALPHA;
            blendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
            blendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
            blendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
        }
        else
        {
            blendState.RenderTarget[0].BlendEnable = false;
            blendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
        }

        CD3DX12_DEPTH_STENCIL_DESC depthStencilState = {};
        depthStencilState.StencilEnable = true;
        depthStencilState.StencilReadMask = 0xff;
        depthStencilState.StencilWriteMask = 0xff;
        depthStencilState.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
        depthStencilState.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
        depthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_INCR_SAT;
        depthStencilState.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
        depthStencilState.BackFace = depthStencilState.FrontFace;

        if (m_Description.EnableDepthTest)
        {
            depthStencilState.DepthEnable = true;
            depthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
            depthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        }
        else
        {
            depthStencilState.DepthEnable = false;
            depthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
            depthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
        }

        //////////////////////////////////////////////////////////////////////
        ///                            Create PSO                           //
        //////////////////////////////////////////////////////////////////////
        Shader* shader = m_Description.Shader.get();
        Device& device = Device::Get();

        m_D3DDescription.InputLayout = inputLayoutDesc;
        m_D3DDescription.pRootSignature = shader->GetResourceLayout().GetRootSignature().Get();
        m_D3DDescription.VS = CD3DX12_SHADER_BYTECODE(shader->GetVSData().Get());
        m_D3DDescription.PS = CD3DX12_SHADER_BYTECODE(shader->GetPSData().Get());
        m_D3DDescription.RasterizerState = rasterizerState;
        m_D3DDescription.BlendState = blendState;
        m_D3DDescription.DepthStencilState = depthStencilState;
        m_D3DDescription.SampleMask = UINT_MAX;
        m_D3DDescription.PrimitiveTopologyType = Utils::AtomTopologyToD3D12(m_Description.Topology);
        m_D3DDescription.NumRenderTargets = AttachmentPoint::NumColorAttachments;

        for (u8 i = 0; i < AttachmentPoint::NumColorAttachments; i++)
        {
            auto colorAttachment = m_Description.Framebuffer->GetColorAttachment((AttachmentPoint)i);
            if (colorAttachment)
            {
                m_D3DDescription.RTVFormats[i] = Utils::AtomTextureFormatToRTVFormat(colorAttachment->GetFormat());
            }
        }

        auto depthAttachment = m_Description.Framebuffer->GetDepthAttachment();
        if (depthAttachment)
        {
            m_D3DDescription.DSVFormat = Utils::AtomTextureFormatToDSVFormat(depthAttachment->GetFormat());
        }

        m_D3DDescription.SampleDesc.Count = 1;

        DXCall(device.GetD3DDevice()->CreateGraphicsPipelineState(&m_D3DDescription, IID_PPV_ARGS(&m_D3DPipeline)));

#if defined (ATOM_DEBUG)
        String name = debugName;
        DXCall(m_D3DPipeline->SetName(STRING_TO_WSTRING(name).c_str()));
#endif
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    GraphicsPipeline::~GraphicsPipeline()
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const PipelineLayout& GraphicsPipeline::GetLayout() const
    {
        return m_Description.Layout;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const Shader* GraphicsPipeline::GetShader() const
    {
        return m_Description.Shader.get();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const Framebuffer* GraphicsPipeline::GetFramebuffer() const
    {
        return m_Description.Framebuffer.get();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Topology GraphicsPipeline::GetTopology() const
    {
        return m_Description.Topology;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool GraphicsPipeline::IsDepthTestingEnabled() const
    {
        return m_Description.EnableDepthTest;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool GraphicsPipeline::IsBlendingEnabled() const
    {
        return m_Description.EnableBlend;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool GraphicsPipeline::IsWireframe() const
    {
        return m_Description.Wireframe;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool GraphicsPipeline::IsBackfaceCullingEnabled() const
    {
        return m_Description.BackfaceCulling;
    }
}
