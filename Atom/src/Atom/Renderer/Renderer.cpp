#include "atompch.h"
#include "Renderer.h"

#include "Atom/Core/Application.h"
#include "Atom/Renderer/CommandBuffer.h"
#include "Atom/Renderer/CommandQueue.h"
#include "Atom/Renderer/Texture.h"
#include "Atom/Renderer/Material.h"
#include "Atom/Renderer/Mesh.h"
#include "Atom/Renderer/Framebuffer.h"
#include "Atom/Renderer/Buffer.h"

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
        ms_ShaderLibrary.Load<GraphicsShader>("resources/shaders/MeshPBRShader.hlsl");
        ms_ShaderLibrary.Load<GraphicsShader>("resources/shaders/SkyBoxShader.hlsl");
        ms_ShaderLibrary.Load<GraphicsShader>("resources/shaders/ImGuiShader.hlsl");
        ms_ShaderLibrary.Load<GraphicsShader>("resources/shaders/CompositeShader.hlsl");
        ms_ShaderLibrary.Load<ComputeShader>("resources/shaders/GenerateMips.hlsl");
        ms_ShaderLibrary.Load<ComputeShader>("resources/shaders/EquirectToCubeMap.hlsl");
        ms_ShaderLibrary.Load<ComputeShader>("resources/shaders/CubeMapPrefilter.hlsl");
        ms_ShaderLibrary.Load<ComputeShader>("resources/shaders/CubeMapIrradiance.hlsl");
        ms_ShaderLibrary.Load<ComputeShader>("resources/shaders/BRDFShader.hlsl");

        // Load pipelines
        {
            FramebufferDescription fbDesc;
            fbDesc.SwapChainFrameBuffer = false;
            fbDesc.Width = 1980;
            fbDesc.Height = 1080;
            fbDesc.ClearColor = { 0.2f, 0.2f, 0.2f, 1.0 };
            fbDesc.Attachments[AttachmentPoint::Color0] = { TextureFormat::RGBA16F, TextureFilter::Linear, TextureWrap::Clamp };
            fbDesc.Attachments[AttachmentPoint::Depth] = { TextureFormat::Depth24Stencil8, TextureFilter::Linear, TextureWrap::Clamp };

            Ref<Framebuffer> frameBuffer = CreateRef<Framebuffer>(fbDesc);

            {
                GraphicsPipelineDescription pipelineDesc;
                pipelineDesc.Topology = Topology::Triangles;
                pipelineDesc.Shader = ms_ShaderLibrary.Get<GraphicsShader>("MeshPBRShader");
                pipelineDesc.Framebuffer = frameBuffer;
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

                ms_PipelineLibrary.Load<GraphicsPipeline>("MeshPBRPipeline", pipelineDesc);
            }

            {
                GraphicsPipelineDescription pipelineDesc;
                pipelineDesc.Topology = Topology::Triangles;
                pipelineDesc.Shader = ms_ShaderLibrary.Get<GraphicsShader>("SkyBoxShader");
                pipelineDesc.Framebuffer = frameBuffer;
                pipelineDesc.Layout = {
                    { "POSITION", ShaderDataType::Float3 },
                    { "TEX_COORD", ShaderDataType::Float2 },
                };

                pipelineDesc.EnableBlend = false;
                pipelineDesc.EnableDepthTest = false;
                pipelineDesc.Wireframe = false;
                pipelineDesc.BackfaceCulling = true;

                ms_PipelineLibrary.Load<GraphicsPipeline>("SkyBoxPipeline", pipelineDesc);
            }
        }

        {
            FramebufferDescription fbDesc;
            fbDesc.SwapChainFrameBuffer = false;
            fbDesc.Width = 1980;
            fbDesc.Height = 1080;
            fbDesc.ClearColor = { 0.2f, 0.2f, 0.2f, 1.0 };
            fbDesc.Attachments[AttachmentPoint::Color0] = { TextureFormat::RGBA8, TextureFilter::Linear, TextureWrap::Clamp };

            Ref<Framebuffer> frameBuffer = CreateRef<Framebuffer>(fbDesc);

            {
                GraphicsPipelineDescription pipelineDesc;
                pipelineDesc.Topology = Topology::Triangles;
                pipelineDesc.Shader = ms_ShaderLibrary.Get<GraphicsShader>("CompositeShader");
                pipelineDesc.Framebuffer = frameBuffer;
                pipelineDesc.Layout = {
                    { "POSITION", ShaderDataType::Float3 },
                    { "TEX_COORD", ShaderDataType::Float2 },
                };

                pipelineDesc.EnableBlend = false;
                pipelineDesc.EnableDepthTest = false;
                pipelineDesc.Wireframe = false;
                pipelineDesc.BackfaceCulling = true;

                ms_PipelineLibrary.Load<GraphicsPipeline>("CompositePipeline", pipelineDesc);
            }
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
            pipelineDesc.Shader = ms_ShaderLibrary.Get<ComputeShader>("GenerateMips");

            ms_PipelineLibrary.Load<ComputePipeline>("GenerateMipsPipeline", pipelineDesc);
        }

        {
            ComputePipelineDescription pipelineDesc;
            pipelineDesc.Shader = ms_ShaderLibrary.Get<ComputeShader>("EquirectToCubeMap");

            ms_PipelineLibrary.Load<ComputePipeline>("EquirectToCubeMapPipeline", pipelineDesc);
        }

        {
            ComputePipelineDescription pipelineDesc;
            pipelineDesc.Shader = ms_ShaderLibrary.Get<ComputeShader>("CubeMapPrefilter");

            ms_PipelineLibrary.Load<ComputePipeline>("CubeMapPrefilterPipeline", pipelineDesc);
        }

        {
            ComputePipelineDescription pipelineDesc;
            pipelineDesc.Shader = ms_ShaderLibrary.Get<ComputeShader>("CubeMapIrradiance");

            ms_PipelineLibrary.Load<ComputePipeline>("CubeMapIrradiancePipeline", pipelineDesc);
        }

        {
            ComputePipelineDescription pipelineDesc;
            pipelineDesc.Shader = ms_ShaderLibrary.Get<ComputeShader>("BRDFShader");

            ms_PipelineLibrary.Load<ComputePipeline>("BRDFPipeline", pipelineDesc);
        }

        // Create fullscreen quad buffers
        struct QuadVertex
        {
            glm::vec3 Position;
            glm::vec2 TexCoord;

            QuadVertex(f32 x, f32 y, f32 z, f32 u, f32 v)
                : Position(x, y, z), TexCoord(u, v)
            {}
        };

        QuadVertex quadVertices[] = {
            QuadVertex(-1.0, -1.0, 0.0, 0.0, 1.0),
            QuadVertex( 1.0, -1.0, 0.0, 1.0, 1.0),
            QuadVertex( 1.0,  1.0, 0.0, 1.0, 0.0),
            QuadVertex(-1.0,  1.0, 0.0, 0.0, 0.0)
        };

        BufferDescription vbDesc;
        vbDesc.ElementCount = _countof(quadVertices);
        vbDesc.ElementSize = sizeof(QuadVertex);
        vbDesc.IsDynamic = false;

        ms_FullscreenQuadVB = CreateRef<VertexBuffer>(vbDesc, "FullscreenQuadVB(Renderer)");

        u16 quadIndices[] = { 0, 1, 2, 2, 3, 0 };

        BufferDescription ibDesc;
        ibDesc.ElementCount = _countof(quadIndices);
        ibDesc.ElementSize = sizeof(u16);
        ibDesc.IsDynamic = false;

        ms_FullscreenQuadIB = CreateRef<IndexBuffer>(ibDesc, IndexBufferFormat::U16, "FullscreenQuadIB(Renderer)");

        // Upload the data to the GPU
        CommandQueue* copyQueue = Device::Get().GetCommandQueue(CommandQueueType::Copy);
        {
            Ref<CommandBuffer> copyCommandBuffer = copyQueue->GetCommandBuffer();
            copyCommandBuffer->Begin();
            copyCommandBuffer->UploadBufferData(quadVertices, ms_FullscreenQuadVB.get());
            copyCommandBuffer->UploadBufferData(quadIndices, ms_FullscreenQuadIB.get());
            copyCommandBuffer->End();
            copyQueue->ExecuteCommandList(copyCommandBuffer);
        }

        // Create error texture
        TextureDescription errorTextureDesc;
        errorTextureDesc.Width = 1;
        errorTextureDesc.Height = 1;
        errorTextureDesc.Format = TextureFormat::RGBA8;
        errorTextureDesc.MipLevels = 1;

        ms_ErrorTexture = CreateRef<Texture2D>(errorTextureDesc, "ErrorTexture(Renderer)");

        u32 errorTextureData = 0xFFFF00FF; // Just 1 pixel of pink color
        {
            Ref<CommandBuffer> copyCommandBuffer = copyQueue->GetCommandBuffer();
            copyCommandBuffer->Begin();
            copyCommandBuffer->UploadTextureData(&errorTextureData, ms_ErrorTexture.get());
            copyCommandBuffer->End();
            copyQueue->ExecuteCommandList(copyCommandBuffer);
        }

        // Create black texture and black texture cube
        TextureDescription blackTextureDesc;
        blackTextureDesc.Width = 1;
        blackTextureDesc.Height = 1;
        blackTextureDesc.Format = TextureFormat::RGBA8;
        blackTextureDesc.MipLevels = 1;

        ms_BlackTexture = CreateRef<Texture2D>(blackTextureDesc, "BlackTexture(Renderer)");
        ms_BlackTextureCube = CreateRef<TextureCube>(blackTextureDesc, "BlackTextureCube(Renderer)");

        u32 blackTextureData = 0xFF000000; // Just 1 pixel of black color
        {
            Ref<CommandBuffer> copyCommandBuffer = copyQueue->GetCommandBuffer();
            copyCommandBuffer->Begin();
            copyCommandBuffer->UploadTextureData(&blackTextureData, ms_BlackTexture.get());

            for (u32 arraySlice = 0; arraySlice < 6; arraySlice++)
                copyCommandBuffer->UploadTextureData(&blackTextureData, ms_BlackTextureCube.get(), 0, arraySlice);

            copyCommandBuffer->End();
            copyQueue->ExecuteCommandList(copyCommandBuffer);
        }

        // Generate BRDF texture
        TextureDescription brdfDesc;
        brdfDesc.Width = 256;
        brdfDesc.Height = 256;
        brdfDesc.Format = TextureFormat::RG16F;
        brdfDesc.MipLevels = 1;
        brdfDesc.UsageFlags = TextureBindFlags::UnorderedAccess;
        brdfDesc.Wrap = TextureWrap::Clamp;

        ms_BRDFTexture = CreateRef<Texture2D>(brdfDesc, "BRDFTexture(Renderer)");

        u32 currentFrameIdx = GetCurrentFrameIndex();
        D3D12_GPU_DESCRIPTOR_HANDLE resourceTable = ms_ResourceHeaps[currentFrameIdx]->CopyDescriptor(ms_BRDFTexture->GetUAV());

        CommandQueue* computeQueue = Device::Get().GetCommandQueue(CommandQueueType::Compute);
        Ref<CommandBuffer> computeCmdBuffer = computeQueue->GetCommandBuffer();
        computeCmdBuffer->Begin();
        computeCmdBuffer->TransitionResource(ms_BRDFTexture.get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        computeCmdBuffer->SetComputePipeline(ms_PipelineLibrary.Get<ComputePipeline>("BRDFPipeline").get());
        computeCmdBuffer->SetDescriptorHeaps(ms_ResourceHeaps[currentFrameIdx].get(), nullptr);
        computeCmdBuffer->SetComputeDescriptorTable(0, resourceTable);
        computeCmdBuffer->Dispatch(glm::max(brdfDesc.Width / 32, 1u), glm::max(brdfDesc.Height / 32, 1u), 1);
        computeCmdBuffer->TransitionResource(ms_BRDFTexture.get(), D3D12_RESOURCE_STATE_COMMON);
        computeCmdBuffer->End();
        computeQueue->ExecuteCommandList(computeCmdBuffer);

        // Wait for all copy/compute operations to complete before we continue
        computeQueue->Flush();
        copyQueue->Flush();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::Shutdown()
    {
        ms_PipelineLibrary.Clear();
        ms_ShaderLibrary.Clear();
        ms_ResourceHeaps.clear();
        ms_SamplerHeaps.clear();
        ms_FullscreenQuadVB.reset();
        ms_FullscreenQuadIB.reset();
        ms_BRDFTexture.reset();
        ms_ErrorTexture.reset();
        ms_BlackTexture.reset();
        ms_BlackTextureCube.reset();
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
    void Renderer::BeginRenderPass(Ref<CommandBuffer> commandBuffer, Ref<Framebuffer> framebuffer, bool clear)
    {
        commandBuffer->BeginRenderPass(framebuffer.get(), clear);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::EndRenderPass(Ref<CommandBuffer> commandBuffer, Ref<Framebuffer> framebuffer)
    {
        commandBuffer->EndRenderPass(framebuffer.get());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::RenderGeometry(Ref<CommandBuffer> commandBuffer, Ref<GraphicsPipeline> pipeline, Ref<Mesh> mesh, Ref<ConstantBuffer> constantBuffer, Ref<StructuredBuffer> structuredBuffer)
    {
        commandBuffer->SetGraphicsPipeline(pipeline.get());
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
            if (constantBuffer)
            {
                commandBuffer->SetGraphicsConstantBuffer(currentRootParameter++, constantBuffer.get());
            }

            if (structuredBuffer)
            {
                commandBuffer->SetGraphicsStructuredBuffer(currentRootParameter++, structuredBuffer.get());
            }

            // Set textures and samplers
            Vector<D3D12_CPU_DESCRIPTOR_HANDLE> textureSRVs;
            textureSRVs.reserve(material->GetTextures().size());

            Vector<D3D12_CPU_DESCRIPTOR_HANDLE> samplers;
            samplers.reserve(material->GetTextures().size());

            for (const auto& texture : material->GetTextures())
            {
                if (!texture)
                {
                    commandBuffer->TransitionResource(ms_ErrorTexture.get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
                    textureSRVs.push_back(ms_ErrorTexture->GetSRV());
                    samplers.push_back(ms_ErrorTexture->GetSampler());
                }
                else
                {
                    commandBuffer->TransitionResource(texture.get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
                    textureSRVs.push_back(texture->GetSRV());
                    samplers.push_back(texture->GetSampler());
                }
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
    void Renderer::RenderFullscreenQuad(Ref<CommandBuffer> commandBuffer, Ref<GraphicsPipeline> pipeline, Ref<ConstantBuffer> constantBuffer, Ref<Material> material)
    {
        commandBuffer->SetGraphicsPipeline(pipeline.get());
        commandBuffer->SetVertexBuffer(ms_FullscreenQuadVB.get());
        commandBuffer->SetIndexBuffer(ms_FullscreenQuadIB.get());

        u32 currentFrameIndex = GetCurrentFrameIndex();
        commandBuffer->SetDescriptorHeaps(ms_ResourceHeaps[currentFrameIndex].get(), ms_SamplerHeaps[currentFrameIndex].get());

        u32 currentRootParameter = 0;

        // Set root constants
        for (const auto& [bufferSlot, data] : material->GetUniformBuffersData())
        {
            commandBuffer->SetGraphicsRootConstants(currentRootParameter++, data.data(), data.size() / 4);
        }

        // Set constant buffers and structured buffers
        if(constantBuffer)
        {
            commandBuffer->SetGraphicsConstantBuffer(currentRootParameter++, constantBuffer.get());
        }

        // Set textures and samplers
        Vector<D3D12_CPU_DESCRIPTOR_HANDLE> textureSRVs;
        textureSRVs.reserve(material->GetTextures().size());

        Vector<D3D12_CPU_DESCRIPTOR_HANDLE> samplers;
        samplers.reserve(material->GetTextures().size());

        for (const auto& texture : material->GetTextures())
        {
            if (!texture)
            {
                commandBuffer->TransitionResource(ms_ErrorTexture.get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
                textureSRVs.push_back(ms_ErrorTexture->GetSRV());
                samplers.push_back(ms_ErrorTexture->GetSampler());
            }
            else
            {
                commandBuffer->TransitionResource(texture.get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
                textureSRVs.push_back(texture->GetSRV());
                samplers.push_back(texture->GetSampler());
            }
        }

        D3D12_GPU_DESCRIPTOR_HANDLE texturesDescriptorTable = ms_ResourceHeaps[currentFrameIndex]->CopyDescriptors(textureSRVs.data(), textureSRVs.size());
        commandBuffer->SetGraphicsDescriptorTable(currentRootParameter++, texturesDescriptorTable);

        D3D12_GPU_DESCRIPTOR_HANDLE samplerDescriptorTable = ms_SamplerHeaps[currentFrameIndex]->CopyDescriptors(samplers.data(), samplers.size());
        commandBuffer->SetGraphicsDescriptorTable(currentRootParameter++, samplerDescriptorTable);

        // Draw
        commandBuffer->DrawIndexed(ms_FullscreenQuadIB->GetElementCount(), 1, 0, 0, 0);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    std::pair<Ref<TextureCube>, Ref<TextureCube>> Renderer::CreateEnvironmentMap(const std::filesystem::path& filepath)
    {
        u32 currentFrameIdx = GetCurrentFrameIndex();
        Ref<DescriptorHeap> currentResourceHeap = ms_ResourceHeaps[currentFrameIdx];
        Ref<DescriptorHeap> currentSamplerHeap = ms_SamplerHeaps[currentFrameIdx];

        CommandQueue* computeQueue = Device::Get().GetCommandQueue(CommandQueueType::Compute);
        CommandQueue* copyQueue = Device::Get().GetCommandQueue(CommandQueueType::Copy);

        /////////////////////////////////////////////////////////////////////////////////////////////////
        //                               Phase 0: Load the hdr texture                                 //
        /////////////////////////////////////////////////////////////////////////////////////////////////

        Image2D hdrImage(filepath);

        TextureDescription textureDesc;
        textureDesc.Width = hdrImage.GetWidth();
        textureDesc.Height = hdrImage.GetHeight();
        textureDesc.Format = TextureFormat::RGBA32F;
        textureDesc.MipLevels = hdrImage.GetMaxMipCount();
        textureDesc.UsageFlags = TextureBindFlags::UnorderedAccess;

        String name = filepath.stem().string();
        Ref<Texture2D> hdrMap = CreateRef<Texture2D>(textureDesc, name.c_str());

        {
            Ref<CommandBuffer> copyCmdBuffer = copyQueue->GetCommandBuffer();
            copyCmdBuffer->Begin();
            copyCmdBuffer->UploadTextureData(hdrImage.GetPixelData().data(), hdrMap.get());
            copyCmdBuffer->End();
            copyQueue->ExecuteCommandList(copyCmdBuffer);
        }

        Renderer::GenerateMips(hdrMap);

        /////////////////////////////////////////////////////////////////////////////////////////////////
        //                        Phase 1: Equirectangular map to cubemap                              //
        /////////////////////////////////////////////////////////////////////////////////////////////////

        TextureDescription envMapDesc;
        envMapDesc.Width = 1024;
        envMapDesc.Height = 1024;
        envMapDesc.Format = TextureFormat::RGBA16F;
        envMapDesc.MipLevels = 11;
        envMapDesc.UsageFlags = TextureBindFlags::UnorderedAccess;

        Ref<TextureCube> envMapUnfiltered = CreateRef<TextureCube>(envMapDesc, fmt::format("{}(EnvironmentMapUnfiltered)", name).c_str());

        {
            PIXBeginEvent(computeQueue->GetD3DCommandQueue().Get(), 0, "EquirectToCubeMap");

            D3D12_GPU_DESCRIPTOR_HANDLE samplerTable = currentSamplerHeap->CopyDescriptor(hdrMap->GetSampler());

            Ref<CommandBuffer> computeCmdBuffer = computeQueue->GetCommandBuffer();
            computeCmdBuffer->Begin();
            computeCmdBuffer->TransitionResource(hdrMap.get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            computeCmdBuffer->TransitionResource(envMapUnfiltered.get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            computeCmdBuffer->SetComputePipeline(ms_PipelineLibrary.Get<ComputePipeline>("EquirectToCubeMapPipeline").get());
            computeCmdBuffer->SetDescriptorHeaps(currentResourceHeap.get(), currentSamplerHeap.get());
            computeCmdBuffer->SetComputeDescriptorTable(2, samplerTable);

            for (u32 mip = 0; mip < envMapDesc.MipLevels; mip++)
            {
                D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptors[] = { hdrMap->GetSRV(), envMapUnfiltered->GetArrayUAV(mip) };
                D3D12_GPU_DESCRIPTOR_HANDLE resourceTable = currentResourceHeap->CopyDescriptors(cpuDescriptors, _countof(cpuDescriptors));

                computeCmdBuffer->SetComputeRootConstants(0, &mip, 1);
                computeCmdBuffer->SetComputeDescriptorTable(1, resourceTable);
                computeCmdBuffer->Dispatch(envMapDesc.Width / 32, envMapDesc.Height / 32, 6);
            }

            computeCmdBuffer->TransitionResource(envMapUnfiltered.get(), D3D12_RESOURCE_STATE_COMMON);
            computeCmdBuffer->End();
            computeQueue->WaitForQueue(copyQueue);
            computeQueue->ExecuteCommandList(computeCmdBuffer);

            PIXEndEvent(computeQueue->GetD3DCommandQueue().Get());
        }

        /////////////////////////////////////////////////////////////////////////////////////////////////
        //                                    Phase 2: Pre-filtering                                   //
        /////////////////////////////////////////////////////////////////////////////////////////////////

        Ref<TextureCube> envMap = CreateRef<TextureCube>(envMapDesc, fmt::format("{}(EnvironmentMap)", name).c_str());

        // Copy the first mip of every face from the unfiltered map
        {
            Ref<CommandBuffer> copyCmdBuffer = copyQueue->GetCommandBuffer();
            copyCmdBuffer->Begin();

            for (u32 arraySlice = 0; arraySlice < 6; arraySlice++)
            {
                u32 subresourceIdx = D3D12CalcSubresource(0, arraySlice, 0, envMapDesc.MipLevels, 6);
                copyCmdBuffer->CopyTexture(envMapUnfiltered.get(), envMap.get(), subresourceIdx);
            }

            copyCmdBuffer->End();
            copyQueue->WaitForQueue(computeQueue);
            copyQueue->ExecuteCommandList(copyCmdBuffer);
        }

        {
            PIXBeginEvent(computeQueue->GetD3DCommandQueue().Get(), 0, "CubeMapPreFilter");

            D3D12_GPU_DESCRIPTOR_HANDLE samplerTable = currentSamplerHeap->CopyDescriptor(envMapUnfiltered->GetSampler());

            Ref<CommandBuffer> computeCmdBuffer = computeQueue->GetCommandBuffer();
            computeCmdBuffer->Begin();
            computeCmdBuffer->TransitionResource(envMapUnfiltered.get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            computeCmdBuffer->TransitionResource(envMap.get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            computeCmdBuffer->SetComputePipeline(ms_PipelineLibrary.Get<ComputePipeline>("CubeMapPrefilterPipeline").get());
            computeCmdBuffer->SetDescriptorHeaps(currentResourceHeap.get(), currentSamplerHeap.get());
            computeCmdBuffer->SetComputeDescriptorTable(2, samplerTable);

            u32 width = glm::max(envMapDesc.Width / 2, 1u);
            u32 height = glm::max(envMapDesc.Height / 2, 1u);

            for (u32 mip = 1; mip < envMapDesc.MipLevels; mip++)
            {
                D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptors[] = { envMapUnfiltered->GetSRV(), envMap->GetArrayUAV(mip) };
                D3D12_GPU_DESCRIPTOR_HANDLE resourceTable = currentResourceHeap->CopyDescriptors(cpuDescriptors, _countof(cpuDescriptors));

                f32 roughness = mip / glm::max(envMapDesc.MipLevels - 1.0f, 1.0f);
                computeCmdBuffer->SetComputeRootConstants(0, &roughness, 1);
                computeCmdBuffer->SetComputeDescriptorTable(1, resourceTable);
                computeCmdBuffer->Dispatch(glm::max(width / 32, 1u), glm::max(height / 32, 1u), 6);

                width = glm::max(width / 2, 1u);
                height = glm::max(height / 2, 1u);
            }

            computeCmdBuffer->End();
            computeQueue->WaitForQueue(copyQueue);
            computeQueue->ExecuteCommandList(computeCmdBuffer);

            PIXEndEvent(computeQueue->GetD3DCommandQueue().Get());
        }

        /////////////////////////////////////////////////////////////////////////////////////////////////
        //                                    Phase 3: Irradiance map                                  //
        /////////////////////////////////////////////////////////////////////////////////////////////////

        TextureDescription irradianceMapDesc;
        irradianceMapDesc.Width = 32;
        irradianceMapDesc.Height = 32;
        irradianceMapDesc.Format = TextureFormat::RGBA16F;
        irradianceMapDesc.MipLevels = 1;
        irradianceMapDesc.UsageFlags = TextureBindFlags::UnorderedAccess;

        Ref<TextureCube> irradianceMap = CreateRef<TextureCube>(irradianceMapDesc, fmt::format("{}(IrradianceMap)", name).c_str());

        D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptors[] = { envMap->GetSRV(), irradianceMap->GetArrayUAV() };
        D3D12_GPU_DESCRIPTOR_HANDLE resourceTable = currentResourceHeap->CopyDescriptors(cpuDescriptors, _countof(cpuDescriptors));
        D3D12_GPU_DESCRIPTOR_HANDLE samplerTable = currentSamplerHeap->CopyDescriptor(envMap->GetSampler());

        PIXBeginEvent(computeQueue->GetD3DCommandQueue().Get(), 0, "GenerateIrradianceMap");

        Ref<CommandBuffer> computeCmdBuffer = computeQueue->GetCommandBuffer();
        computeCmdBuffer->Begin();
        computeCmdBuffer->TransitionResource(envMap.get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        computeCmdBuffer->TransitionResource(irradianceMap.get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        computeCmdBuffer->SetComputePipeline(ms_PipelineLibrary.Get<ComputePipeline>("CubeMapIrradiancePipeline").get());
        computeCmdBuffer->SetDescriptorHeaps(currentResourceHeap.get(), currentSamplerHeap.get());
        computeCmdBuffer->SetComputeDescriptorTable(0, resourceTable);
        computeCmdBuffer->SetComputeDescriptorTable(1, samplerTable);
        computeCmdBuffer->Dispatch(glm::max(irradianceMapDesc.Width / 32, 1u), glm::max(irradianceMapDesc.Height / 32, 1u), 6);
        computeCmdBuffer->End();
        computeQueue->ExecuteCommandList(computeCmdBuffer);

        PIXEndEvent(computeQueue->GetD3DCommandQueue().Get());

        // Wait for all compute operations to complete
        computeQueue->Flush();

        return std::pair<Ref<TextureCube>, Ref<TextureCube>>(envMap, irradianceMap);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::GenerateMips(Ref<Texture2D> texture)
    {
        CommandQueue* computeQueue = Device::Get().GetCommandQueue(CommandQueueType::Compute);
        PIXBeginEvent(computeQueue->GetD3DCommandQueue().Get(), 0, "GenerateMips");

        u32 currentFrameIdx = GetCurrentFrameIndex();
        Ref<DescriptorHeap> currentResourceHeap = ms_ResourceHeaps[currentFrameIdx];
        Ref<DescriptorHeap> currentSamplerHeap = ms_SamplerHeaps[currentFrameIdx];

        // Create bilinear clamp sampler to use for mip generation
        D3D12_SAMPLER_DESC samplerDesc = {};
        samplerDesc.Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
        samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        samplerDesc.MipLODBias = 0.0f;
        samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        samplerDesc.MinLOD = 0.0f;
        samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
        samplerDesc.MaxAnisotropy = 0;

        auto& dx12Device = Device::Get();
        D3D12_CPU_DESCRIPTOR_HANDLE sampler = dx12Device.GetCPUDescriptorHeap(DescriptorHeapType::Sampler)->AllocateDescriptor();
        dx12Device.GetD3DDevice()->CreateSampler(&samplerDesc, sampler);

        // Copy the sampler CPU descriptor into the GPU visible heap
        D3D12_GPU_DESCRIPTOR_HANDLE samplerTable = currentSamplerHeap->CopyDescriptor(sampler);

        // Run compute shader for each mip
        Ref<CommandBuffer> computeCmdBuffer = computeQueue->GetCommandBuffer();
        computeCmdBuffer->Begin();
        computeCmdBuffer->SetComputePipeline(ms_PipelineLibrary.Get<ComputePipeline>("GenerateMipsPipeline").get());
        computeCmdBuffer->SetDescriptorHeaps(currentResourceHeap.get(), currentSamplerHeap.get());
        computeCmdBuffer->SetComputeDescriptorTable(2, samplerTable);

        u32 width = glm::max(texture->GetWidth() / 2, 1u);
        u32 height = glm::max(texture->GetHeight() / 2, 1u);

        struct GenerateMipCB
        {
            glm::vec2 TexelSize;
            u32 TopMipLevel;
        };

        for (u32 mip = 1; mip < texture->GetMipLevels(); mip++)
        {
            D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptors[] = { texture->GetSRV(), texture->GetUAV(mip) };
            D3D12_GPU_DESCRIPTOR_HANDLE resourceTable = currentResourceHeap->CopyDescriptors(cpuDescriptors, _countof(cpuDescriptors));

            GenerateMipCB constants;
            constants.TexelSize = { 1.0f / width, 1.0f / height };
            constants.TopMipLevel = mip - 1;

            computeCmdBuffer->TransitionResource(texture.get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, mip - 1);
            computeCmdBuffer->TransitionResource(texture.get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, mip);
            computeCmdBuffer->SetComputeRootConstants(0, &constants, 3);
            computeCmdBuffer->SetComputeDescriptorTable(1, resourceTable);
            computeCmdBuffer->Dispatch(glm::max(width / 8, 1u), glm::max(height / 8, 1u), 1);

            computeCmdBuffer->AddUAVBarrier(texture.get());

            width = glm::max(width / 2, 1u);
            height = glm::max(height / 2, 1u);
        }

        computeCmdBuffer->End();

        // Wait for any copy operations to complate in order to make sure we have all the data already uploaded before executing
        computeQueue->WaitForQueue(Device::Get().GetCommandQueue(CommandQueueType::Copy));
        computeQueue->ExecuteCommandList(computeCmdBuffer);

        // Wait for the compute operations to finish before continuing
        computeQueue->Flush();

        PIXEndEvent(computeQueue->GetD3DCommandQueue().Get());
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

    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<Texture2D> Renderer::GetBRDF()
    {
        return ms_BRDFTexture;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<Texture2D> Renderer::GetErrorTexture()
    {
        return ms_ErrorTexture;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<Texture2D> Renderer::GetBlackTexture()
    {
        return ms_BlackTexture;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<TextureCube> Renderer::GetBlackTextureCube()
    {
        return ms_BlackTextureCube;
    }
}