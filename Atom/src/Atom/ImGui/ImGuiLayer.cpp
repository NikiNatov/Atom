#include "atompch.h"
#include "ImGuiLayer.h"

#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx12.h>

#include "Atom/Core/Application.h"
#include "Atom/Renderer/Device.h"
#include "Atom/Renderer/Renderer.h"
#include "Atom/Renderer/Texture.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    ImGuiLayer::ImGuiLayer()
        : Layer("ImGuiLayer")
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    ImGuiLayer::~ImGuiLayer()
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ImGuiLayer::OnAttach()
    {
        m_FontSrvHeap = CreateRef<DescriptorHeap>(DescriptorHeapType::ShaderResource, 1, true, "ImGui Font SRV Heap");
        m_CommandBuffer = CreateRef<CommandBuffer>(CommandQueueType::Graphics, "ImGui Command Buffer");

        FramebufferDescription fbDesc;
        fbDesc.SwapChainFrameBuffer = true;
        fbDesc.Width = 1980;
        fbDesc.Height = 1080;
        fbDesc.ClearColor = { 0.2f, 0.2f, 0.2f, 1.0 };
        fbDesc.Attachments[AttachmentPoint::Color0] = { TextureFormat::RGBA8, TextureFilter::Linear, TextureWrap::Clamp };

        m_FrameBuffer = CreateRef<Framebuffer>(fbDesc);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        SetDarkTheme();

        auto& style = ImGui::GetStyle();
        auto& app = Application::Get();

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        ImGui_ImplWin32_Init(app.GetWindow().GetWindowHandle());
        ImGui_ImplDX12_Init(Device::Get().GetD3DDevice().Get(), Renderer::GetFramesInFlight(), 
            DXGI_FORMAT_R8G8B8A8_UNORM, m_FontSrvHeap->GetD3DHeap().Get(), m_FontSrvHeap->GetCPUStartHandle(), m_FontSrvHeap->GetGPUStartHandle());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ImGuiLayer::OnDetach()
    {
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ImGuiLayer::OnImGuiRender()
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ImGuiLayer::OnUpdate(Timestep ts)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ImGuiLayer::OnEvent(Event& event)
    {
        if (m_BlockEvents)
        {
            ImGuiIO& io = ImGui::GetIO();
            event.Handled |= event.IsInCategory(EventCategoryMouse) & io.WantCaptureMouse;
            event.Handled |= event.IsInCategory(EventCategoryKeyboard) & io.WantCaptureKeyboard;
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ImGuiLayer::BeginFrame()
    {
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ImGuiLayer::EndFrame()
    {
        m_CommandBuffer->Begin();

        m_CommandBuffer->BeginRenderPass(m_FrameBuffer.get(), false);
        m_CommandBuffer->GetCommandList()->SetDescriptorHeaps(1, m_FontSrvHeap->GetD3DHeap().GetAddressOf());
        ImGui::Render();
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_CommandBuffer->GetCommandList().Get());

        auto& app = Application::Get();
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2((f32)app.GetWindow().GetWidth(), (f32)app.GetWindow().GetHeight());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }

        m_CommandBuffer->EndRenderPass(m_FrameBuffer.get());
        m_CommandBuffer->End();
        auto fence = Device::Get().GetCommandQueue(CommandQueueType::Graphics)->ExecuteCommandList(m_CommandBuffer.get());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ImGuiLayer::SetBlockEvents(bool block)
    {
        m_BlockEvents = block;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ImGuiLayer::SetDarkTheme()
    {
        auto& colors = ImGui::GetStyle().Colors;

        colors[ImGuiCol_Header] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
        colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
        colors[ImGuiCol_HeaderActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

        // Buttons
        colors[ImGuiCol_Button] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
        colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
        colors[ImGuiCol_ButtonActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

        // Frame BG
        colors[ImGuiCol_FrameBg] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
        colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
        colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

        // Tabs
        colors[ImGuiCol_Tab] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
        colors[ImGuiCol_TabHovered] = ImVec4{ 0.38f, 0.3805f, 0.381f, 1.0f };
        colors[ImGuiCol_TabActive] = ImVec4{ 0.28f, 0.2805f, 0.281f, 1.0f };
        colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };

        // Title
        colors[ImGuiCol_TitleBg] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
        colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
    }
}
