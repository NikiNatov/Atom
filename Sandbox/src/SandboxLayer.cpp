#include "SandboxLayer.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    SandboxLayer::SandboxLayer()
        : Layer("SandboxLayer")
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    SandboxLayer::~SandboxLayer()
    {
        
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SandboxLayer::OnAttach()
    {
        // Create command buffers
        m_CommandBuffer = CommandBuffer::Create();

        // Create pipeline
        FramebufferDescription fbDesc;
        fbDesc.SwapChainFrameBuffer = true;
        fbDesc.Width = 1980;
        fbDesc.Height = 1080;
        fbDesc.ClearColor[0] = 0.2f;
        fbDesc.ClearColor[1] = 0.2f;
        fbDesc.ClearColor[2] = 0.2f;
        fbDesc.ClearColor[3] = 1.0f;
        fbDesc.Attachments[AttachmentPoint::Color0] = { TextureFormat::RGBA8, TextureFilter::Linear, TextureWrap::Clamp };
        fbDesc.Attachments[AttachmentPoint::DepthStencil] = { TextureFormat::Depth24Stencil8, TextureFilter::Linear, TextureWrap::Clamp };

        GraphicsPipelineDescription pipelineDesc;
        pipelineDesc.Topology = Topology::Triangles;
        pipelineDesc.Shader = Shader::Create("assets/shaders/Shader.hlsl");
        pipelineDesc.Framebuffer = Framebuffer::Create(fbDesc);
        pipelineDesc.Layout = {
            { "POSITION", ShaderDataType::Float3 },
            { "NORMAL", ShaderDataType::Float3 },
            { "TEXCOORD", ShaderDataType::Float2 }
        };
        pipelineDesc.EnableDepthTest = false;
        pipelineDesc.EnableBlend = true;
        pipelineDesc.Wireframe = false;
        pipelineDesc.BackfaceCulling = false;

        m_DefaultPipeline = GraphicsPipeline::Create(pipelineDesc);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SandboxLayer::OnDetach()
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SandboxLayer::OnUpdate(Timestep ts)
    {
        Renderer::BeginFrame(m_CommandBuffer);
        Renderer::BeginRenderPass(m_CommandBuffer, m_DefaultPipeline->GetFramebuffer());
        Renderer::Draw(m_CommandBuffer, m_DefaultPipeline, 3);
        Renderer::EndRenderPass(m_CommandBuffer, m_DefaultPipeline->GetFramebuffer());
        Renderer::EndFrame(m_CommandBuffer);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SandboxLayer::OnImGuiRender()
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SandboxLayer::OnEvent(Event& event)
    {
    }
}