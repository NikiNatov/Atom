#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Core/Layer.h"
#include "Atom/Core/DirectX12/DirectX12.h"

#include "Atom/Renderer/DescriptorHeap.h"

namespace Atom
{
    class CommandBuffer;
    class GraphicsPipeline;
    class Texture2D;
    class Texture;
    class VertexBuffer;
    class IndexBuffer;

    class ImGuiLayer : public Layer
    {
        using TextureCache = HashMap<const Texture*, DescriptorAllocation>;
    public:
        ImGuiLayer();
        virtual ~ImGuiLayer();

        virtual void OnAttach() override;
        virtual void OnDetach() override;
        virtual void OnEvent(Event& event) override;

        void BeginFrame();
        void EndFrame();
        void SetBlockEvents(bool block);
        void SetClearRenderTarget(bool clear);
        void SetDarkTheme();
    private:
        DescriptorAllocation GetTextureHandle(const Texture* texture);
        void SetRenderState(Ref<CommandBuffer> commandBuffer);
        void RenderDrawData();
        void CreateGraphicsObjects();
    private:
        bool                      m_BlockEvents = true;
        bool                      m_ClearRenderTarget = false;
        Ref<GraphicsPipeline>     m_Pipeline;
        Ref<Texture>              m_FontTexture;
        DescriptorAllocation      m_SamplerDescriptor;
        Ref<VertexBuffer>         m_VertexBuffers[g_FramesInFlight];
        Ref<IndexBuffer>          m_IndexBuffers[g_FramesInFlight];
        TextureCache              m_TextureCache[g_FramesInFlight];
    };
}