#include "atompch.h"
#include "Renderer.h"

namespace Atom
{
    Scope<Renderer> Renderer::ms_Instance = CreateScope<Renderer>();

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::Initialize()
    {
        // Create adapter
        ms_Instance->m_Adapter = Adapter::CreateAdapter(AdapterPreference::HighPerformance);
        ATOM_ENGINE_INFO("GPU: {0}", ms_Instance->m_Adapter->GetDescription());
        ATOM_ENGINE_INFO("Video Memory: {0} MB", MB(ms_Instance->m_Adapter->GetVideoMemory()));
        ATOM_ENGINE_INFO("System Memory: {0} MB", MB(ms_Instance->m_Adapter->GetSystemMemory()));
        ATOM_ENGINE_INFO("Shared Memory: {0} MB", MB(ms_Instance->m_Adapter->GetSharedMemory()));

        // Create device
        ms_Instance->m_Device = Device::CreateDevice(ms_Instance->m_Adapter.get());

        // Create command queue
        CommandQueueDesc commandQueueDesc = {};
        commandQueueDesc.Priority = CommandQueuePriority::Normal;
        commandQueueDesc.Type = CommandListType::Direct;

        ms_Instance->m_GraphicsQueue = CommandQueue::CreateCommandQueue(ms_Instance->m_Device.get(), commandQueueDesc);

        // Create allocators
        for (u32 i = 0; i < FRAME_COUNT; i++)
        {
            ms_Instance->m_FrameAllocators[i] = CommandAllocator::CreateCommandAllocator(ms_Instance->m_Device.get(), CommandListType::Direct);
        }

        // Create command list
        ms_Instance->m_GraphicsCommandList = GraphicsCommandList::CreateGraphicsCommandList(ms_Instance->m_Device.get(), CommandListType::Direct, ms_Instance->m_FrameAllocators[0].get());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::BeginFrame()
    {
        u32 frameIndex = ms_Instance->m_CurrentFrameIndex;

        // Wait for the GPU to catch up
        ms_Instance->m_GraphicsQueue->WaitForFenceValue(ms_Instance->m_FrameFenceValues[frameIndex]);

        // Reset the current frame command allocator
        ms_Instance->m_FrameAllocators[frameIndex]->Reset();

        // Reset the graphics command list with the current frame allocator 
        ms_Instance->m_GraphicsCommandList->Reset(ms_Instance->m_FrameAllocators[frameIndex].get());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::EndFrame()
    {
        // Close the command list
        ms_Instance->m_GraphicsCommandList->Close();

        // Execute the commands, signal the command queue and update the frame's fence value
        ms_Instance->m_GraphicsQueue->ExecuteCommandList(ms_Instance->m_GraphicsCommandList.get());
        ms_Instance->m_FrameFenceValues[ms_Instance->m_CurrentFrameIndex] = ms_Instance->m_GraphicsQueue->Signal();

        // Increment frame index
        ms_Instance->m_CurrentFrameIndex = (ms_Instance->m_CurrentFrameIndex + 1) % FRAME_COUNT;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::Flush()
    {
        for (u32 i = 0; i < FRAME_COUNT; i++)
            ms_Instance->m_GraphicsQueue->WaitForFenceValue(ms_Instance->m_FrameFenceValues[i]);

        ms_Instance->m_CurrentFrameIndex = 0;
    }
}