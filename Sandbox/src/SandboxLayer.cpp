#include "SandboxLayer.h"
#include <glm/glm.hpp>

namespace Atom
{
    struct CameraCB
    {
        glm::mat4 ViewMatrix = glm::mat4(1.0f);
        glm::mat4 ProjMatrix = glm::mat4(1.0f);
        f32 p[32] {0};
    };

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

        BufferDescription vbDesc;
        vbDesc.ElementCount = _countof(vertices);
        vbDesc.ElementSize = sizeof(Vertex);
        vbDesc.Data = vertices;

        m_QuadVertexBuffer = CreateRef<VertexBuffer>(vbDesc, "QuadVertexBuffer");

        u32 indices[] = { 0, 1, 2, 2, 3, 0 };

        BufferDescription ibDesc;
        ibDesc.ElementCount = _countof(indices);
        ibDesc.ElementSize = sizeof(u32);
        ibDesc.Data = indices;

        m_QuadIndexBuffer = CreateRef<IndexBuffer>(ibDesc, IndexBufferFormat::U32, "QuadIndexBuffer");

        BufferDescription cbDesc;
        cbDesc.ElementCount = 1;
        cbDesc.ElementSize = sizeof(CameraCB);
        cbDesc.Data = nullptr;

        m_CameraCB = CreateRef<ConstantBuffer>(cbDesc, "CameraCB");
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SandboxLayer::OnDetach()
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SandboxLayer::OnUpdate(Timestep ts)
    {
        m_Camera.OnUpdate(ts);

        CameraCB cameraCB;
        cameraCB.ProjMatrix = m_Camera.GetProjectionMatrix();
        cameraCB.ViewMatrix = m_Camera.GetViewMatrix();

        CommandBuffer* cmdBuffer = m_CommandBuffer.get();
        Renderer::BeginFrame(cmdBuffer);
        m_CameraCB->SetData(cmdBuffer, &cameraCB, sizeof(CameraCB));
        Renderer::BeginRenderPass(cmdBuffer, m_DefaultPipeline->GetFramebuffer());
        Renderer::RenderGeometry(cmdBuffer, m_DefaultPipeline.get(), m_QuadVertexBuffer.get(), m_QuadIndexBuffer.get(), m_CameraCB.get());
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
        m_Camera.OnEvent(event);
    }
}