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
        };
        pipelineDesc.EnableDepthTest = false;
        pipelineDesc.EnableBlend = true;
        pipelineDesc.Wireframe = false;
        pipelineDesc.BackfaceCulling = false;

        m_DefaultPipeline = GraphicsPipeline::Create(pipelineDesc);

        struct Vertex
        {
            f32 x, y, z;

            Vertex(f32 x, f32 y, f32 z)
                : x(x), y(y), z(z)
            {}
        };

        // Create buffers
        Vertex vertices[] = {
            Vertex(-0.5f, -0.5f, 0.0f),
            Vertex( 0.5f, -0.5f, 0.0f),
            Vertex( 0.5f,  0.5f, 0.0f),
            Vertex(-0.5f,  0.5f, 0.0f),
        };

        VertexBufferDescription vbDesc;
        vbDesc.VertexCount = _countof(vertices);
        vbDesc.VertexStride = sizeof(Vertex);
        vbDesc.Data = vertices;

        m_QuadVertexBuffer = VertexBuffer::Create(vbDesc, "QuadVertexBuffer");

        u32 indices[] = { 0, 1, 2, 2, 3, 0 };

        IndexBufferDescription ibDesc;
        ibDesc.IndexCount = _countof(indices);
        ibDesc.Format = IndexBufferFormat::U32;
        ibDesc.Data = indices;

        m_QuadIndexBuffer = IndexBuffer::Create(ibDesc, "QuadIndexBuffer");
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SandboxLayer::OnDetach()
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SandboxLayer::OnUpdate(Timestep ts)
    {
        CommandBuffer* cmdBuffer = m_CommandBuffer.get();
        Renderer::BeginFrame(cmdBuffer);
        Renderer::BeginRenderPass(cmdBuffer, m_DefaultPipeline->GetFramebuffer());
        Renderer::RenderGeometry(cmdBuffer, m_DefaultPipeline.get(), m_QuadVertexBuffer.get(), m_QuadIndexBuffer.get());
        Renderer::EndRenderPass(cmdBuffer, m_DefaultPipeline->GetFramebuffer());
        Renderer::EndFrame(cmdBuffer);
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