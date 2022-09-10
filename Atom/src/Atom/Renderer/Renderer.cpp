#include "atompch.h"
#include "Renderer.h"

#include "Atom/Core/Application.h"
#include "Atom/Renderer/Device.h"
#include "Atom/Renderer/CommandBuffer.h"
#include "Atom/Renderer/CommandQueue.h"
#include "Atom/Renderer/Texture.h"
#include "Atom/Renderer/GraphicsPipeline.h"
#include "Atom/Renderer/Framebuffer.h"
#include "Atom/Renderer/Buffer.h"
#include "Atom/Renderer/Material.h"

namespace Atom
{
    RendererConfig Renderer::ms_Config;
    Vector<Ref<DescriptorHeap>> Renderer::ms_ResourceHeaps;
    Vector<Ref<DescriptorHeap>> Renderer::ms_SamplerHeaps;

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::Initialize(const RendererConfig& config)
    {
        ms_Config = config;
        ms_ResourceHeaps.resize(ms_Config.FramesInFlight);
        ms_SamplerHeaps.resize(ms_Config.FramesInFlight);

        for (u32 i = 0; i < ms_Config.FramesInFlight; i++)
        {
            ms_ResourceHeaps[i] = CreateRef<DescriptorHeap>(DescriptorHeapType::ShaderResource, ms_Config.MaxDescriptorsPerHeap, true,
                fmt::format("ResourceDescriptorHeap[{}]", i).c_str());
        }

        for (u32 i = 0; i < ms_Config.FramesInFlight; i++)
        {
            ms_SamplerHeaps[i] = CreateRef<DescriptorHeap>(DescriptorHeapType::Sampler, ms_Config.MaxDescriptorsPerHeap, true,
                fmt::format("SamplerDescriptorHeap[{}]", i).c_str());
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::BeginFrame()
    {
        u32 currentFrameIndex = GetCurrentFrameIndex();

        Device::Get().ProcessDeferredReleases(currentFrameIndex);
        PIXBeginEvent(Device::Get().GetCommandQueue(CommandQueueType::Graphics)->GetD3DCommandQueue().Get(), 0, "Begin Frame");

        ms_ResourceHeaps[currentFrameIndex]->Reset();
        ms_SamplerHeaps[currentFrameIndex]->Reset();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::BeginRenderPass(CommandBuffer* commandBuffer, const Framebuffer* framebuffer, bool clear)
    {
        commandBuffer->BeginRenderPass(framebuffer, clear);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::EndRenderPass(CommandBuffer* commandBuffer, const Framebuffer* framebuffer)
    {
        commandBuffer->EndRenderPass(framebuffer);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::RenderGeometry(CommandBuffer* commandBuffer, const GraphicsPipeline* pipeline, const VertexBuffer* vertexBuffer, const IndexBuffer* indexBuffer, const ConstantBuffer* constantBuffer, const Material* material)
    {
        commandBuffer->SetGraphicsPipeline(pipeline);
        commandBuffer->SetVertexBuffer(vertexBuffer);
        commandBuffer->SetIndexBuffer(indexBuffer);

        u32 currentFrameIndex = GetCurrentFrameIndex();
        commandBuffer->SetDescriptorHeaps(ms_ResourceHeaps[currentFrameIndex].get(), ms_SamplerHeaps[currentFrameIndex].get());

        u32 currentRootParameter = 0;
        for (const auto& [bufferSlot, data] : material->GetUniformBuffersData())
        {
            commandBuffer->SetGraphicsRootConstants(currentRootParameter++, data.data(), data.size() / 4);
        }

        // TODO: Find a better way of setting constant buffers
        commandBuffer->SetGraphicsConstantBuffer(currentRootParameter++, constantBuffer);

        Vector<D3D12_CPU_DESCRIPTOR_HANDLE> textureSRVs;
        Vector<D3D12_CPU_DESCRIPTOR_HANDLE> samplers;
        textureSRVs.reserve(material->GetTextures().size());
        samplers.reserve(material->GetTextures().size());

        for (const auto& texture : material->GetTextures())
        {
            textureSRVs.push_back(texture->GetSRV());
            samplers.push_back(texture->GetSampler());
        }

        D3D12_GPU_DESCRIPTOR_HANDLE texturesDescriptorTable = ms_ResourceHeaps[currentFrameIndex]->CopyDescriptors(textureSRVs.data(), textureSRVs.size());
        D3D12_GPU_DESCRIPTOR_HANDLE samplerDescriptorTable = ms_SamplerHeaps[currentFrameIndex]->CopyDescriptors(samplers.data(), samplers.size());
        commandBuffer->SetGraphicsDescriptorTable(currentRootParameter++, texturesDescriptorTable);
        commandBuffer->SetGraphicsDescriptorTable(currentRootParameter++, samplerDescriptorTable);

        commandBuffer->DrawIndexed(indexBuffer->GetElementCount());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::EndFrame()
    {
        PIXEndEvent(Device::Get().GetCommandQueue(CommandQueueType::Graphics)->GetD3DCommandQueue().Get());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const RendererConfig& Renderer::GetConfig()
    {
        return ms_Config;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 Renderer::GetCurrentFrameIndex()
    {
        return Application::Get().GetWindow().GetSwapChain()->GetCurrentBackBufferIndex();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 Renderer::GetFramesInFlight()
    {
        return ms_Config.FramesInFlight;
    }
}