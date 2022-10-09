#include "EditorLayer.h"
#include "EditorResources.h"
#include "Panels/ConsolePanel.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
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

        // Set the pipeline
        m_GeometryPipeline = Renderer::GetPipelineLibrary().Get<GraphicsPipeline>("GeometryPipeline");

        // Load the test mesh
        m_TestMesh = CreateRef<Mesh>("assets/meshes/x-wing/x-wing.gltf");

        // Create camera constant buffer
        BufferDescription cbDesc;
        cbDesc.ElementCount = 1;
        cbDesc.ElementSize = sizeof(CameraCB);
        cbDesc.IsDynamic = true;

        m_CameraCB = CreateRef<ConstantBuffer>(cbDesc, "CameraCB");

        // Create compute shader test output texture
        TextureDescription textureDesc;
        textureDesc.Width = 512;
        textureDesc.Height = 512;
        textureDesc.UsageFlags = TextureBindFlags::UnorderedAccess;

        m_ComputeShaderTestTexture = CreateRef<Texture2D>(textureDesc, "ComputeShaderTestOutput");

        EditorResources::Initialize();
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

        void* data = m_CameraCB->Map(0, 0);
        memcpy(data, &cameraCB, sizeof(CameraCB));
        m_CameraCB->Unmap();

        CommandQueue* gfxQueue = Device::Get().GetCommandQueue(CommandQueueType::Graphics);
        Ref<CommandBuffer> commandBuffer = gfxQueue->GetCommandBuffer();
        commandBuffer->Begin();

        Renderer::BeginRenderPass(commandBuffer.get(), m_GeometryPipeline->GetFramebuffer());
        Renderer::RenderGeometry(commandBuffer.get(), m_GeometryPipeline.get(), m_TestMesh.get(), m_CameraCB.get());
        Renderer::EndRenderPass(commandBuffer.get(), m_GeometryPipeline->GetFramebuffer());

        commandBuffer->End();

        gfxQueue->ExecuteCommandList(commandBuffer);
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

        ImGui::Begin("ComputeShaderTestOutput");
        if (ImGui::Button("Run Compute Test"))
        {
            RunComputeShaderTest();
        }
        ImGui::Image((ImTextureID)m_ComputeShaderTestTexture.get(), {(f32)m_ComputeShaderTestTexture->GetWidth(), (f32)m_ComputeShaderTestTexture->GetHeight()});
        ImGui::End();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Viewport");
        ImVec2 panelSize = ImGui::GetContentRegionAvail();

        if (m_ViewportSize.x != panelSize.x || m_ViewportSize.y != panelSize.y)
        {
            m_ViewportSize = { panelSize.x, panelSize.y };
            m_GeometryPipeline->GetFramebuffer()->Resize(m_ViewportSize.x, m_ViewportSize.y);
            m_Camera.SetViewport(m_ViewportSize.x, m_ViewportSize.y);
        }

        const RenderTexture2D* sceneTexture = m_GeometryPipeline->GetFramebuffer()->GetColorAttachment(AttachmentPoint::Color0);
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

    // -----------------------------------------------------------------------------------------------------------------------------
    void EditorLayer::RunComputeShaderTest()
    {
        struct TextureDimensionsCB
        {
            u32 Width;
            u32 Height;
        };

        TextureDimensionsCB Constants;
        Constants.Width = m_ComputeShaderTestTexture->GetWidth();
        Constants.Height = m_ComputeShaderTestTexture->GetHeight();

        Ref<DescriptorHeap> heap = CreateRef<DescriptorHeap>(DescriptorHeapType::ShaderResource, 1, true, "ComputeShaderTestHeap");
        D3D12_GPU_DESCRIPTOR_HANDLE outputTextureHandle = heap->CopyDescriptor(m_ComputeShaderTestTexture->GetUAV());

        // Transition the texture
        CommandQueue* gfxQueue = Device::Get().GetCommandQueue(CommandQueueType::Graphics);
        Ref<CommandBuffer> gfxCmdBuffer = gfxQueue->GetCommandBuffer();
        gfxCmdBuffer->Begin();
        gfxCmdBuffer->TransitionResource(m_ComputeShaderTestTexture.get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        gfxCmdBuffer->End();
        gfxQueue->ExecuteCommandList(gfxCmdBuffer);

        CommandQueue* computeQueue = Device::Get().GetCommandQueue(CommandQueueType::Compute);
        Ref<CommandBuffer> computeCmdBuffer = computeQueue->GetCommandBuffer();
        computeCmdBuffer->Begin();

        computeCmdBuffer->SetComputePipeline(Renderer::GetPipelineLibrary().Get<ComputePipeline>("TestComputePipeline").get());
        computeCmdBuffer->SetDescriptorHeaps(heap.get(), nullptr);
        computeCmdBuffer->SetComputeRootConstants(0, &Constants, 2);
        computeCmdBuffer->SetComputeDescriptorTable(1, outputTextureHandle);
        computeCmdBuffer->Dispatch(Constants.Width / 32, Constants.Height / 32, 1);

        computeCmdBuffer->End();

        // Wait for the gfx queue to execute the transitions cmd list
        computeQueue->WaitForQueue(gfxQueue);
        computeQueue->ExecuteCommandList(computeCmdBuffer);
        computeQueue->Flush();
    }
}