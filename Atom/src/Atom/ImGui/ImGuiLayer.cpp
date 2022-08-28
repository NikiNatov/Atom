#include "atompch.h"
#include "ImGuiLayer.h"

#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx12.h>

#include "Atom/Core/Application.h"
#include "Atom/Renderer/Renderer.h"
#include "Atom/Renderer/Texture.h"
#include "Atom/Renderer/DescriptorHeap.h"
#include "Atom/Renderer/CommandBuffer.h"
#include "Atom/Renderer/GraphicsPipeline.h"
#include "Atom/Renderer/Buffer.h"
#include "Atom/Renderer/Framebuffer.h"

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
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        SetDarkTheme();

        ImGui_ImplWin32_Init(Application::Get().GetWindow().GetWindowHandle());
        CreateGraphicsObjects();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ImGuiLayer::OnDetach()
    {
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
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
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ImGuiLayer::EndFrame()
    {
        ImGui::Render();
        RenderDrawData();

        auto& app = Application::Get();
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2((f32)app.GetWindow().GetWidth(), (f32)app.GetWindow().GetHeight());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ImGuiLayer::SetBlockEvents(bool block)
    {
        m_BlockEvents = block;
    }

    void ImGuiLayer::SetClearRenderTarget(bool clear)
    {
        m_ClearRenderTarget = clear;
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

    // -----------------------------------------------------------------------------------------------------------------------------
    D3D12_GPU_DESCRIPTOR_HANDLE ImGuiLayer::GetTextureHandle(const Texture* texture)
    {
        if (m_TextureRegistry.find(texture) == m_TextureRegistry.end())
        {
            D3D12_GPU_DESCRIPTOR_HANDLE gpuDescriptor = m_GPUDescriptorHeap->CopyDescriptor(texture->GetSRV());
            m_TextureRegistry.emplace(texture, gpuDescriptor);
            return gpuDescriptor;
        }

        return m_TextureRegistry[texture];
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ImGuiLayer::SetRenderState()
    {
        struct TransformCB
        {
            f32 MVPMatrix[4][4];
        };

        ImDrawData* drawData = ImGui::GetDrawData();

        TransformCB transformCB;
        f32 L = drawData->DisplayPos.x;
        f32 R = drawData->DisplayPos.x + drawData->DisplaySize.x;
        f32 T = drawData->DisplayPos.y;
        f32 B = drawData->DisplayPos.y + drawData->DisplaySize.y;
        f32 mvp[4][4] =
        {
            { 2.0f / (R - L),     0.0f,               0.0f,       0.0f },
            { 0.0f,               2.0f / (T - B),     0.0f,       0.0f },
            { 0.0f,               0.0f,               0.5f,       0.0f },
            { (R + L) / (L - R),  (T + B) / (B - T),  0.5f,       1.0f },
        };
        memcpy(&transformCB.MVPMatrix, mvp, sizeof(mvp));

        u32 currentFrameIndex = Renderer::GetCurrentFrameIndex();
        m_CommandBuffer->SetVertexBuffer(m_VertexBuffers[currentFrameIndex].get());
        m_CommandBuffer->SetIndexBuffer(m_IndexBuffers[currentFrameIndex].get());
        m_CommandBuffer->SetGraphicsPipeline(m_Pipeline.get());
        m_CommandBuffer->SetDescriptorHeaps(m_GPUDescriptorHeap.get(), m_SamplerDescriptorHeap.get());
        m_CommandBuffer->SetGraphicsRootConstants(0, &transformCB, 16);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ImGuiLayer::RenderDrawData()
    {
        ImDrawData* drawData = ImGui::GetDrawData();

        if (drawData->DisplaySize.x > 0.0f && drawData->DisplaySize.y > 0.0f)
        {
            m_CommandBuffer->Begin();
            m_CommandBuffer->BeginRenderPass(m_Pipeline->GetFramebuffer(), m_ClearRenderTarget);

            u32 currentFrameIndex = Renderer::GetCurrentFrameIndex();

            // Create and grow vertex/index buffers if needed
            if (m_VertexBuffers[currentFrameIndex] == nullptr || m_VertexBuffers[currentFrameIndex]->GetElementCount() < drawData->TotalVtxCount)
            {
                BufferDescription vbDesc;
                vbDesc.ElementCount = drawData->TotalVtxCount + 5000;
                vbDesc.ElementSize = sizeof(ImDrawVert);
                vbDesc.IsDynamic = true;

                m_VertexBuffers[currentFrameIndex] = CreateRef<VertexBuffer>(vbDesc, "ImGuiVertexBuffer");
            }

            if (m_IndexBuffers[currentFrameIndex] == nullptr || m_IndexBuffers[currentFrameIndex]->GetElementCount() < drawData->TotalIdxCount)
            {
                BufferDescription ibDesc;
                ibDesc.ElementCount = drawData->TotalIdxCount + 10000;
                ibDesc.ElementSize = sizeof(ImDrawIdx);
                ibDesc.IsDynamic = true;

                m_IndexBuffers[currentFrameIndex] = CreateRef<IndexBuffer>(ibDesc, sizeof(ImDrawIdx) == 2 ? IndexBufferFormat::U16 : IndexBufferFormat::U32, "ImGuiIndexBuffer");
            }

            // Upload vertex/index data into a single contiguous GPU buffer
            void* vbData = m_VertexBuffers[currentFrameIndex]->Map(0, 0);
            void* ibData = m_IndexBuffers[currentFrameIndex]->Map(0, 0);

            ImDrawVert* vtxDst = (ImDrawVert*)vbData;
            ImDrawIdx* idxDst = (ImDrawIdx*)ibData;

            for (u32 n = 0; n < drawData->CmdListsCount; n++)
            {
                const ImDrawList* cmdList = drawData->CmdLists[n];
                memcpy(vtxDst, cmdList->VtxBuffer.Data, cmdList->VtxBuffer.Size * sizeof(ImDrawVert));
                memcpy(idxDst, cmdList->IdxBuffer.Data, cmdList->IdxBuffer.Size * sizeof(ImDrawIdx));
                vtxDst += cmdList->VtxBuffer.Size;
                idxDst += cmdList->IdxBuffer.Size;
            }

            m_VertexBuffers[currentFrameIndex]->Unmap();
            m_IndexBuffers[currentFrameIndex]->Unmap();

            // Setup the rendering pipeline
            SetRenderState();

            // Render command lists
            u32 globalVtxOffset = 0;
            u32 globalIdxOffset = 0;
            ImVec2 clipOff = drawData->DisplayPos;

            for (u32 n = 0; n < drawData->CmdListsCount; n++)
            {
                const ImDrawList* cmdList = drawData->CmdLists[n];

                for (u32 cmdIdx = 0; cmdIdx < cmdList->CmdBuffer.Size; cmdIdx++)
                {
                    const ImDrawCmd* pcmd = &cmdList->CmdBuffer[cmdIdx];
                    if (pcmd->UserCallback != NULL)
                    {
                        // User callback, registered via ImDrawList::AddCallback()
                        // (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
                        if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
                            SetRenderState();
                        else
                            pcmd->UserCallback(cmdList, pcmd);
                    }
                    else
                    {
                        // Project scissor/clipping rectangles into framebuffer space
                        ImVec2 clipMin(pcmd->ClipRect.x - clipOff.x, pcmd->ClipRect.y - clipOff.y);
                        ImVec2 clipMax(pcmd->ClipRect.z - clipOff.x, pcmd->ClipRect.w - clipOff.y);

                        if (clipMax.x <= clipMin.x || clipMax.y <= clipMin.y)
                            continue;

                        // Apply Scissor/clipping rectangle, Bind texture, Draw
                        const D3D12_RECT r = { (LONG)clipMin.x, (LONG)clipMin.y, (LONG)clipMax.x, (LONG)clipMax.y };
                        Texture* texture = (Texture*)pcmd->GetTexID();
                        m_CommandBuffer->GetCommandList()->RSSetScissorRects(1, &r);
                        m_CommandBuffer->TransitionResource(texture, D3D12_RESOURCE_STATE_GENERIC_READ);
                        m_CommandBuffer->SetGraphicsDescriptorTable(1, GetTextureHandle(texture));
                        m_CommandBuffer->SetGraphicsDescriptorTable(2, m_SamplerDescriptorHeap->GetGPUStartHandle());
                        m_CommandBuffer->DrawIndexed(pcmd->ElemCount, 1, pcmd->IdxOffset + globalIdxOffset, pcmd->VtxOffset + globalVtxOffset);
                    }
                }

                globalIdxOffset += cmdList->IdxBuffer.Size;
                globalVtxOffset += cmdList->VtxBuffer.Size;
            }

            m_CommandBuffer->EndRenderPass(m_Pipeline->GetFramebuffer());
            m_CommandBuffer->End();

            Device::Get().GetCommandQueue(CommandQueueType::Graphics)->ExecuteCommandList(m_CommandBuffer.get());
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ImGuiLayer::CreateGraphicsObjects()
    {
        // Create command buffer
        m_CommandBuffer = CreateRef<CommandBuffer>(CommandQueueType::Graphics, "ImGuiCommandBuffer");

        // Create descriptor heaps
        m_GPUDescriptorHeap = CreateRef<DescriptorHeap>(DescriptorHeapType::ShaderResource, Renderer::GetConfig().MaxDescriptorsPerHeap, true, "ImGuiResourceDescriptorHeap");
        m_SamplerDescriptorHeap = CreateRef<DescriptorHeap>(DescriptorHeapType::Sampler, 1, true, "ImGuiSamplerDescriptorHeap");

        D3D12_SAMPLER_DESC defaultSamplerDesc = {};
        defaultSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
        defaultSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        defaultSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        defaultSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        defaultSamplerDesc.MipLODBias = 0.f;
        defaultSamplerDesc.MaxAnisotropy = 0;
        defaultSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
        defaultSamplerDesc.MinLOD = 0.f;
        defaultSamplerDesc.MaxLOD = 0.f;

        Device::Get().GetD3DDevice()->CreateSampler(&defaultSamplerDesc, m_SamplerDescriptorHeap->GetCPUStartHandle());

        // Create vertex and index buffers;
        m_VertexBuffers.resize(Renderer::GetFramesInFlight(), nullptr);
        m_IndexBuffers.resize(Renderer::GetFramesInFlight(), nullptr);

        // Create pipeline
        FramebufferDescription fbDesc;
        fbDesc.SwapChainFrameBuffer = true;

        GraphicsPipelineDescription pipelineDesc;
        pipelineDesc.Framebuffer = CreateRef<Framebuffer>(fbDesc);
        pipelineDesc.Shader = CreateRef<Shader>("../Atom/assets/shaders/ImGuiShader.hlsl");
        pipelineDesc.EnableBlend = true;
        pipelineDesc.EnableDepthTest = false;
        pipelineDesc.BackfaceCulling = false;
        pipelineDesc.Wireframe = false;
        pipelineDesc.Topology = Topology::Triangles;
        pipelineDesc.Layout = {
            { "POSITION", ShaderDataType::Float2 },
            { "TEX_COORD", ShaderDataType::Float2 },
            { "COLOR", ShaderDataType::Unorm4 },
        };

        m_Pipeline = CreateRef<GraphicsPipeline>(pipelineDesc, "ImGuiPipeline");

        // Create font texture
        ImGuiIO& io = ImGui::GetIO();
        byte* pixels;
        s32 width, height;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

        TextureDescription fontTextureDesc;
        fontTextureDesc.Format = TextureFormat::RGBA8;
        fontTextureDesc.Filter = TextureFilter::Anisotropic;
        fontTextureDesc.Wrap = TextureWrap::Repeat;
        fontTextureDesc.UsageFlags = TextureBindFlags::None;
        fontTextureDesc.Width = width;
        fontTextureDesc.Height = height;

        m_FontTexture = CreateRef<Texture2D>(fontTextureDesc, "ImGuiFontTexture");

        CommandQueue* copyQueue = Device::Get().GetCommandQueue(CommandQueueType::Copy);
        Ref<CommandBuffer> copyCommandBuffer = CreateRef<CommandBuffer>(CommandQueueType::Copy, "ImGuiCopyCommandBuffer");
        copyCommandBuffer->Begin();
        copyCommandBuffer->UploadTextureData(pixels, m_FontTexture.get());
        copyCommandBuffer->End();
        copyQueue->ExecuteCommandList(copyCommandBuffer.get());

        io.Fonts->SetTexID((ImTextureID)m_FontTexture.get());

        // Wait until all copy operations are finished before rendering
        copyQueue->Flush();
    }
}
