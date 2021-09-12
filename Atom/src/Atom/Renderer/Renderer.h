#pragma once

#include "Atom/Core/Core.h"

#include "Adapter.h"
#include "Device.h"
#include "CommandQueue.h"
#include "GraphicsCommandList.h"

namespace Atom
{
    class Renderer
    {
    public:
        Renderer() = default;
        Renderer(const Renderer& other) = delete;
        Renderer& operator=(const Renderer& other) = delete;

        static void Initialize();
        static void BeginFrame();
        static void EndFrame();
        static void Flush();

        static const Adapter* GetAdapter() { return ms_Instance->m_Adapter.get(); }
        static const Device* GetDevice() { return ms_Instance->m_Device.get(); }
        static const CommandQueue* GetGraphicsQueue() { return ms_Instance->m_GraphicsQueue.get(); }
        static u32 GetFrameCount() { return FRAME_COUNT; }
    private:
        static Scope<Renderer>     ms_Instance;
        static constexpr u32       FRAME_COUNT = 3;
    private:
        Scope<Adapter>             m_Adapter;
        Scope<Device>              m_Device;

        Scope<CommandQueue>        m_GraphicsQueue;
        Scope<GraphicsCommandList> m_GraphicsCommandList;

        Scope<CommandAllocator>    m_FrameAllocators[FRAME_COUNT];
        u64                        m_FrameFenceValues[FRAME_COUNT]{ 0 };
        u64                        m_CurrentFrameIndex = 0;
        
    };
}