#include "EditorLayer.h"
#include "EditorResources.h"
#include "Panels/ConsolePanel.h"

#include <glm/glm.hpp>
#include <imgui/imgui.h>

namespace Atom
{
    struct CameraCB
    {
        glm::mat4 ViewMatrix = glm::mat4(1.0f);
        glm::mat4 ProjMatrix = glm::mat4(1.0f);
        f32 p[32]{ 0 };
    };

    // -----------------------------------------------------------------------------------------------------------------------------
    EditorLayer::EditorLayer()
        : Layer("EditorLayer")
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    EditorLayer::~EditorLayer()
    {

    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void EditorLayer::OnAttach()
    {
        Application::Get().GetImGuiLayer().SetClearRenderTarget(true);

        // Create command buffers
        m_CommandBuffer = CreateRef<CommandBuffer>(CommandQueueType::Graphics);

        // Create pipeline
        FramebufferDescription fbDesc;
        fbDesc.SwapChainFrameBuffer = false;
        fbDesc.Width = 1980;
        fbDesc.Height = 1080;
        fbDesc.ClearColor = { 0.2f, 0.2f, 0.2f, 1.0 };
        fbDesc.Attachments[AttachmentPoint::Color0] = { TextureFormat::RGBA8, TextureFilter::Linear, TextureWrap::Clamp };
        fbDesc.Attachments[AttachmentPoint::Depth] = { TextureFormat::Depth24Stencil8, TextureFilter::Linear, TextureWrap::Clamp };

        GraphicsPipelineDescription pipelineDesc;
        pipelineDesc.Topology = Topology::Triangles;
        pipelineDesc.Shader = CreateRef<Shader>("../Atom/shaders/Shader.hlsl");
        pipelineDesc.Framebuffer = CreateRef<Framebuffer>(fbDesc);
        pipelineDesc.Layout = {
            { "POSITION", ShaderDataType::Float3 },
            { "TEX_COORD", ShaderDataType::Float2 },
        };
        pipelineDesc.EnableDepthTest = false;
        pipelineDesc.EnableBlend = true;
        pipelineDesc.Wireframe = false;
        pipelineDesc.BackfaceCulling = false;

        m_DefaultPipeline = CreateRef<GraphicsPipeline>(pipelineDesc);

        struct Vertex
        {
            glm::vec3 Position;
            glm::vec2 UV;

            Vertex(f32 x, f32 y, f32 z, f32 u, f32 v)
                : Position(x, y, z), UV(u, v)
            {}
        };

        // Create buffers
        Vertex vertices[] = {
            Vertex(-0.5f, -0.5f, 0.0f, 0.0f, 1.0f),
            Vertex(0.5f, -0.5f, 0.0f, 1.0f, 1.0f),
            Vertex(0.5f,  0.5f, 0.0f, 1.0f, 0.0f),
            Vertex(-0.5f,  0.5f, 0.0f, 0.0f, 0.0f),
        };

        BufferDescription vbDesc;
        vbDesc.ElementCount = _countof(vertices);
        vbDesc.ElementSize = sizeof(Vertex);
        vbDesc.IsDynamic = false;

        m_QuadVertexBuffer = CreateRef<VertexBuffer>(vbDesc, "QuadVertexBuffer");

        u32 indices[] = { 0, 1, 2, 2, 3, 0 };

        BufferDescription ibDesc;
        ibDesc.ElementCount = _countof(indices);
        ibDesc.ElementSize = sizeof(u32);
        ibDesc.IsDynamic = false;

        m_QuadIndexBuffer = CreateRef<IndexBuffer>(ibDesc, IndexBufferFormat::U32, "QuadIndexBuffer");

        // Upload data to the buffers
        CommandQueue* copyQueue = Device::Get().GetCommandQueue(CommandQueueType::Copy);
        Ref<CommandBuffer> copyCommandBuffer = CreateRef<CommandBuffer>(CommandQueueType::Copy, "CopyCommandBuffer");
        copyCommandBuffer->Begin();
        copyCommandBuffer->UploadBufferData(vertices, m_QuadVertexBuffer.get());
        copyCommandBuffer->UploadBufferData(indices, m_QuadIndexBuffer.get());
        copyCommandBuffer->End();
        copyQueue->ExecuteCommandList(copyCommandBuffer.get());

        BufferDescription cbDesc;
        cbDesc.ElementCount = 1;
        cbDesc.ElementSize = sizeof(CameraCB);
        cbDesc.IsDynamic = true;

        m_CameraCB = CreateRef<ConstantBuffer>(cbDesc, "CameraCB");

        EditorResources::Initialize();

        // Create material
        m_Material = CreateRef<Material>(m_DefaultPipeline->GetShader(), MaterialFlags::None, "Material");
        m_Material->SetTexture("Texture1", EditorResources::ErrorIcon);
        m_Material->SetTexture("Texture2", EditorResources::WarningIcon);
        m_Material->SetTexture("Texture3", EditorResources::InfoIcon);

        ATOM_INFO("Test Info");
        ATOM_WARNING("Test Warning");
        ATOM_ERROR("Test Error");
        ATOM_CRITICAL("Test Critical");
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void EditorLayer::OnDetach()
    {
        EditorResources::Shutdown();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void EditorLayer::OnUpdate(Timestep ts)
    {
        static f32 elapsedTime = 0.0f;
        elapsedTime += ts;

        m_Camera.OnUpdate(ts);

        CameraCB cameraCB;
        cameraCB.ProjMatrix = m_Camera.GetProjectionMatrix();
        cameraCB.ViewMatrix = m_Camera.GetViewMatrix();

        CommandBuffer* cmdBuffer = m_CommandBuffer.get();
        cmdBuffer->Begin();

        void* data = m_CameraCB->Map(0, 0);
        memcpy(data, &cameraCB, sizeof(CameraCB));
        m_CameraCB->Unmap();

        //m_Material->SetUniform("Red", sin(elapsedTime) * 0.5f + 0.5f);
        //m_Material->SetUniform("Green", cos(elapsedTime) * 0.5f + 0.5f);
        //m_Material->SetUniform("Blue", sin(elapsedTime) * 0.5f + 0.5f);

        Renderer::BeginRenderPass(cmdBuffer, m_DefaultPipeline->GetFramebuffer());
        Renderer::RenderGeometry(cmdBuffer, m_DefaultPipeline.get(), m_QuadVertexBuffer.get(), m_QuadIndexBuffer.get(), m_CameraCB.get(), m_Material.get());
        Renderer::EndRenderPass(cmdBuffer, m_DefaultPipeline->GetFramebuffer());
        cmdBuffer->End();

        Device::Get().GetCommandQueue(CommandQueueType::Graphics)->ExecuteCommandList(cmdBuffer);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void EditorLayer::OnImGuiRender()
    {
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DockSpace", 0, window_flags);
        ImGui::PopStyleVar();

        ImGui::PopStyleVar(2);

        // Submit the DockSpace
        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            ImGuiID dockspace_id = ImGui::GetID("EditorDockspace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        }

        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New", "Ctrl+N"))
                {
                }

                if (ImGui::MenuItem("Open Scene...", "Ctrl+O"))
                {
                }

                if (ImGui::MenuItem("Save As...", "Ctrl+S"))
                {
                }

                if (ImGui::MenuItem("Exit"))
                    Application::Get().Close();

                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }

        ConsolePanel::OnImGuiRender();
        ImGui::ShowDemoWindow(false);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Viewport");
        ImVec2 panelSize = ImGui::GetContentRegionAvail();

        if (m_ViewportSize.x != panelSize.x || m_ViewportSize.y != panelSize.y)
        {
            m_ViewportSize = { panelSize.x, panelSize.y };
            m_DefaultPipeline->GetFramebuffer()->Resize(m_ViewportSize.x, m_ViewportSize.y);
            m_Camera.SetViewport(m_ViewportSize.x, m_ViewportSize.y);
        }

        const RenderTexture2D* sceneTexture = m_DefaultPipeline->GetFramebuffer()->GetColorAttachment(AttachmentPoint::Color0);
        ImGui::Image((ImTextureID)sceneTexture, { (f32)sceneTexture->GetWidth(), (f32)sceneTexture->GetHeight() });

        ImGui::End();
        ImGui::PopStyleVar();

        ImGui::End();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void EditorLayer::OnEvent(Event& event)
    {
        m_Camera.OnEvent(event);
    }
}