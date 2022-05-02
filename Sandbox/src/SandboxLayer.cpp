#include "SandboxLayer.h"
#include <glm/glm.hpp>

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
        m_CommandBuffer = CreateRef<CommandBuffer>();

        // Create pipeline
        FramebufferDescription fbDesc;
        fbDesc.SwapChainFrameBuffer = true;
        fbDesc.Width = 1980;
        fbDesc.Height = 1080;
        fbDesc.ClearColor = { 0.2f, 0.2f, 0.2f, 1.0 };
        fbDesc.Attachments[AttachmentPoint::Color0] = { TextureFormat::RGBA8, TextureFilter::Linear, TextureWrap::Clamp };
        fbDesc.Attachments[AttachmentPoint::Depth] = { TextureFormat::Depth24Stencil8, TextureFilter::Linear, TextureWrap::Clamp };

        GraphicsPipelineDescription pipelineDesc;
        pipelineDesc.Topology = Topology::Triangles;
        pipelineDesc.Shader = CreateRef<Shader>("assets/shaders/Shader.hlsl");
        pipelineDesc.Framebuffer = CreateRef<Framebuffer>(fbDesc);
        pipelineDesc.Layout = {
            { "POSITION", ShaderDataType::Float3 },
        };
        pipelineDesc.EnableDepthTest = false;
        pipelineDesc.EnableBlend = true;
        pipelineDesc.Wireframe = false;
        pipelineDesc.BackfaceCulling = false;

        m_DefaultPipeline = CreateRef<GraphicsPipeline>(pipelineDesc);

        struct Vertex
        {
            glm::vec3 Position;

            Vertex(f32 x, f32 y, f32 z)
                : Position(x, y, z)
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

        m_QuadVertexBuffer = CreateRef<VertexBuffer>(vbDesc, "QuadVertexBuffer");

        u32 indices[] = { 0, 1, 2, 2, 3, 0 };

        IndexBufferDescription ibDesc;
        ibDesc.IndexCount = _countof(indices);
        ibDesc.Format = IndexBufferFormat::U32;
        ibDesc.Data = indices;

        m_QuadIndexBuffer = CreateRef<IndexBuffer>(ibDesc, "QuadIndexBuffer");
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