#pragma once

#include "Atom/Core/Layer.h"
#include "Atom/Renderer/DescriptorHeap.h"
#include "Atom/Renderer/CommandBuffer.h"
#include "Atom/Renderer/Framebuffer.h"

namespace Atom
{
    class ImGuiLayer : public Layer
    {
    public:
        ImGuiLayer();
        virtual ~ImGuiLayer();

        virtual void OnAttach() override;
        virtual void OnDetach() override;
        virtual void OnImGuiRender() override;
        virtual void OnUpdate(Timestep ts) override;
        virtual void OnEvent(Event& event) override;

        void BeginFrame();
        void EndFrame();
        void SetBlockEvents(bool block);
        void SetDarkTheme();
    private:
        bool                m_BlockEvents = true;
        Ref<DescriptorHeap> m_FontSrvHeap;
        Ref<CommandBuffer>  m_CommandBuffer;
        Ref<Framebuffer>    m_FrameBuffer;
    };
}