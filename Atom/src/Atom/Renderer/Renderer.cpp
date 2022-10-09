#include "atompch.h"
#include "Renderer.h"

#include "Atom/Core/Application.h"
#include "Atom/Renderer/CommandBuffer.h"
#include "Atom/Renderer/CommandQueue.h"
#include "Atom/Renderer/Texture.h"
#include "Atom/Renderer/Material.h"
#include "Atom/Renderer/Mesh.h"
#include "Atom/Renderer/Framebuffer.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::Initialize(const RendererConfig& config)
    {
        ms_Config = config;
        ms_ResourceHeaps.resize(ms_Config.FramesInFlight);
        ms_SamplerHeaps.resize(ms_Config.FramesInFlight);

        // Create descriptor heaps
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

        // Load shaders
        ms_ShaderLibrary.Load<GraphicsShader>("resources/shaders/Shader.hlsl");
        ms_ShaderLibrary.Load<GraphicsShader>("resources/shaders/ImGuiShader.hlsl");
        ms_ShaderLibrary.Load<ComputeShader>("resources/shaders/ComputeShaderTest.hlsl");

        // Load pipelines
        {
            FramebufferDescription fbDesc;
            fbDesc.SwapChainFrameBuffer = false;
            fbDesc.Width = 1980;
            fbDesc.Height = 1080;
            fbDesc.ClearColor = { 0.2f, 0.2f, 0.2f, 1.0 };
            fbDesc.Attachments[AttachmentPoint::Color0] = { TextureFormat::RGBA8, TextureFilter::Linear, TextureWrap::Clamp };
            fbDesc.Attachments[AttachmentPoint::Depth] = { TextureFormat::Depth24Stencil8, TextureFilter::Linear, TextureWrap::Clamp };

            GraphicsPipelineDescription pipelineDesc;
            pipelineDesc.Topology = Topology::Triangles;
            pipelineDesc.Shader = ms_ShaderLibrary.Get<GraphicsShader>("Shader");
            pipelineDesc.Framebuffer = CreateRef<Framebuffer>(fbDesc);
            pipelineDesc.Layout = {
                { "POSITION", ShaderDataType::Float3 },
                { "TEX_COORD", ShaderDataType::Float2 },
                { "NORMAL", ShaderDataType::Float3 },
                { "TANGENT", ShaderDataType::Float3 },
                { "BITANGENT", ShaderDataType::Float3 },
            };

            pipelineDesc.EnableBlend = true;
            pipelineDesc.EnableDepthTest = true;
            pipelineDesc.Wireframe = false;
            pipelineDesc.BackfaceCulling = true;

            ms_PipelineLibrary.Load<GraphicsPipeline>("GeometryPipeline", pipelineDesc);
        }

        {
            FramebufferDescription fbDesc;
            fbDesc.SwapChainFrameBuffer = true;

            GraphicsPipelineDescription pipelineDesc;
            pipelineDesc.Framebuffer = CreateRef<Framebuffer>(fbDesc);
            pipelineDesc.Shader = ms_ShaderLibrary.Get<GraphicsShader>("ImGuiShader");
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

            ms_PipelineLibrary.Load<GraphicsPipeline>("ImGuiPipeline", pipelineDesc);
        }

        {
            ComputePipelineDescription pipelineDesc;
            pipelineDesc.Shader = ms_ShaderLibrary.Get<ComputeShader>("ComputeShaderTest");

            ms_PipelineLibrary.Load<ComputePipeline>("TestComputePipeline", pipelineDesc);
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::Shutdown()
    {
        ms_PipelineLibrary.Clear();
        ms_ShaderLibrary.Clear();
        ms_ResourceHeaps.clear();
        ms_SamplerHeaps.clear();
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
    void Renderer::RenderGeometry(CommandBuffer* commandBuffer, const GraphicsPipeline* pipeline, const Mesh* mesh, const ConstantBuffer* constantBuffer)
    {
        commandBuffer->SetGraphicsPipeline(pipeline);
        commandBuffer->SetVertexBuffer(mesh->GetVertexBuffer().get());
        commandBuffer->SetIndexBuffer(mesh->GetIndexBuffer().get());

        u32 currentFrameIndex = GetCurrentFrameIndex();
        commandBuffer->SetDescriptorHeaps(ms_ResourceHeaps[currentFrameIndex].get(), ms_SamplerHeaps[currentFrameIndex].get());

        for (auto& submesh : mesh->GetSubmeshes())
        {
            Ref<Material> material = mesh->GetMaterials()[submesh.MaterialIndex];

            u32 currentRootParameter = 0;

            // Set root constants
            for (const auto& [bufferSlot, data] : material->GetUniformBuffersData())
            {
                commandBuffer->SetGraphicsRootConstants(currentRootParameter++, data.data(), data.size() / 4);
            }

            // Set constant buffers and structured buffers
            commandBuffer->SetGraphicsConstantBuffer(currentRootParameter++, constantBuffer);

            // Set textures and samplers
            Vector<D3D12_CPU_DESCRIPTOR_HANDLE> textureSRVs;
            textureSRVs.reserve(material->GetTextures().size());

            Vector<D3D12_CPU_DESCRIPTOR_HANDLE> samplers;
            samplers.reserve(material->GetTextures().size());

            for (const auto& texture : material->GetTextures())
            {
                textureSRVs.push_back(texture->GetSRV());
                samplers.push_back(texture->GetSampler());
            }

            D3D12_GPU_DESCRIPTOR_HANDLE texturesDescriptorTable = ms_ResourceHeaps[currentFrameIndex]->CopyDescriptors(textureSRVs.data(), textureSRVs.size());
            commandBuffer->SetGraphicsDescriptorTable(currentRootParameter++, texturesDescriptorTable);

            D3D12_GPU_DESCRIPTOR_HANDLE samplerDescriptorTable = ms_SamplerHeaps[currentFrameIndex]->CopyDescriptors(samplers.data(), samplers.size());
            commandBuffer->SetGraphicsDescriptorTable(currentRootParameter++, samplerDescriptorTable);

            // Draw
            commandBuffer->DrawIndexed(submesh.IndexCount, 1, submesh.StartIndex, submesh.StartVertex, 0);
        }
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

    // -----------------------------------------------------------------------------------------------------------------------------
    const ShaderLibrary& Renderer::GetShaderLibrary()
    {
        return ms_ShaderLibrary;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const PipelineLibrary& Renderer::GetPipelineLibrary()
    {
        return ms_PipelineLibrary;
    }
}