#pragma once

#include "Atom/Core/Layer.h"
#include "Atom/Core/DirectX12/DirectX12.h"

namespace Atom
{
    class CommandBuffer;
    class GraphicsPipeline;
    class DescriptorHeap;
    class Texture2D;
    class Texture;
    class VertexBuffer;
    class IndexBuffer;

    class ImGuiLayer : public Layer
    {
        using TextureCache = HashMap<const Texture*, D3D12_GPU_DESCRIPTOR_HANDLE>;
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
        D3D12_GPU_DESCRIPTOR_HANDLE GetTextureHandle(const Texture* texture);
        void SetRenderState();
        void RenderDrawData();
        void CreateGraphicsObjects();
    private:
        bool                        m_BlockEvents = true;
        bool                        m_ClearRenderTarget = false;
        Ref<CommandBuffer>          m_CommandBuffer;
        Ref<GraphicsPipeline>       m_Pipeline;
        Vector<Ref<DescriptorHeap>> m_GPUDescriptorHeaps;
        Ref<DescriptorHeap>         m_SamplerDescriptorHeap;
        Ref<Texture2D>              m_FontTexture;
        Vector<Ref<VertexBuffer>>   m_VertexBuffers;
        Vector<Ref<IndexBuffer>>    m_IndexBuffers;
        Vector<TextureCache>        m_TextureCache;
    };
}