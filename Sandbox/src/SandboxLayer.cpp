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
        m_CommandBuffer = CommandBuffer::Create();

        TextureDescription desc;
        desc.Width = 512;
        desc.Height = 512;
        m_Texture = Texture2D::Create(desc);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SandboxLayer::OnDetach()
    {
        //m_Texture->DeferredRelease();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SandboxLayer::OnUpdate(Timestep ts)
    {
        Renderer::BeginFrame(m_CommandBuffer);

        Renderer::EndFrame(m_CommandBuffer);
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