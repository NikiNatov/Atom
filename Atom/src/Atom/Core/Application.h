#pragma once

#include "Events/Events.h"
#include "LayerStack.h"
#include "Timer.h"
#include "Window.h"

#include "Atom/ImGui/ImGuiLayer.h"

#include "Atom/Renderer/Renderer.h"
#include "Atom/Renderer/ShaderLibrary.h"
#include "Atom/Renderer/PipelineLibrary.h"

namespace Atom
{
    struct CommandLineArgs
    {
        s32 Count = 0;
        char** Args = nullptr;

        const char* operator[](u32 index) const
        {
            ATOM_ENGINE_ASSERT(index < Count);
            return Args[index];
        }
    };

    struct ApplicationSpecification
    {
        String              Name = "Atom Application";
        CommandLineArgs     CommandLineArgs;
        u32                 WindowWidth = 1280;
        u32                 WindowHeight = 720;
        bool                VSync = true;
        Vector<SinkWrapper> AppLoggerSinks;
    };

    class Application
    {
    public:
        Application(const ApplicationSpecification& spec);
        virtual ~Application();

        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;

        void Run();
        void OnEvent(Event& event);
        void Close();
        void PushLayer(Layer* layer);
        void PushOverlay(Layer* overlay);

        inline const ApplicationSpecification& GetSpecification() { return m_Specification; }
        inline ShaderLibrary& GetShaderLibrary() { return *m_ShaderLibrary; }
        inline PipelineLibrary& GetPipelineLibrary() { return *m_PipelineLibrary; }
        inline Window& GetWindow() { return *m_Window; }
        inline ImGuiLayer& GetImGuiLayer() { return *m_ImGuiLayer; }
        inline u32 GetFPS() const { return m_FPS; }
        inline u32 GetCurrentFrameIndex() const { return m_Window->GetSwapChain()->GetCurrentBackBufferIndex(); }

        void SubmitForMainThreadExecution(const std::function<void()>& function);
    private:
        bool OnWindowClosed(WindowClosedEvent& event);

        void ExecuteMainThreadQueue();
    public:
        static inline Application& Get() { return *ms_Application; }
    private:
        ApplicationSpecification m_Specification;
        bool                     m_Running = true;
        Timer                    m_FrameTimer;
        Scope<Window>            m_Window;
        Scope<ShaderLibrary>     m_ShaderLibrary;
        Scope<PipelineLibrary>   m_PipelineLibrary;
        LayerStack               m_LayerStack;
        ImGuiLayer*              m_ImGuiLayer;
        u32                      m_FPS = 0;

        Vector<std::function<void()>> m_MainThreadQueue;
        std::mutex m_MainThreadQueueMutex;
    private:
        inline static Application* ms_Application = nullptr;
    };

    Application* CreateApplication(const CommandLineArgs& args);
}

