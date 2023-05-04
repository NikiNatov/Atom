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
#include "Atom/Renderer/LightEnvironment.h"

#include "Atom/Tools/ContentTools.h"

#include <pix3.h>

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::Initialize(const RendererConfig& config)
    {
        ms_Config = config;

        // Load shaders
        ms_ShaderLibrary.Load<GraphicsShader>("resources/shaders/MeshPBRShader.hlsl");
        ms_ShaderLibrary.Load<GraphicsShader>("resources/shaders/MeshPBRAnimatedShader.hlsl");
        ms_ShaderLibrary.Load<GraphicsShader>("resources/shaders/SkyBoxShader.hlsl");
        ms_ShaderLibrary.Load<GraphicsShader>("resources/shaders/ImGuiShader.hlsl");
        ms_ShaderLibrary.Load<GraphicsShader>("resources/shaders/CompositeShader.hlsl");
        ms_ShaderLibrary.Load<GraphicsShader>("resources/shaders/FullscreenQuadShader.hlsl");
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

            Ref<Framebuffer> frameBuffer = CreateRef<Framebuffer>(fbDesc, "GeometryFramebuffer");

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
                pipelineDesc.Shader = ms_ShaderLibrary.Get<GraphicsShader>("MeshPBRAnimatedShader");
                pipelineDesc.Framebuffer = frameBuffer;
                pipelineDesc.Layout = {
                    { "POSITION", ShaderDataType::Float3 },
                    { "TEX_COORD", ShaderDataType::Float2 },
                    { "NORMAL", ShaderDataType::Float3 },
                    { "TANGENT", ShaderDataType::Float3 },
                    { "BITANGENT", ShaderDataType::Float3 },
                    { "BONE_IDS", ShaderDataType::Uint4 },
                    { "BONE_WEIGHTS", ShaderDataType::Float4 },
                };
                pipelineDesc.EnableBlend = true;
                pipelineDesc.EnableDepthTest = true;
                pipelineDesc.Wireframe = false;
                pipelineDesc.BackfaceCulling = true;

                ms_PipelineLibrary.Load<GraphicsPipeline>("MeshPBRAnimatedPipeline", pipelineDesc);
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

            Ref<Framebuffer> frameBuffer = CreateRef<Framebuffer>(fbDesc, "CompositeFramebuffer");

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

            {
                GraphicsPipelineDescription pipelineDesc;
                pipelineDesc.Topology = Topology::Triangles;
                pipelineDesc.Shader = ms_ShaderLibrary.Get<GraphicsShader>("FullscreenQuadShader");
                pipelineDesc.Framebuffer = frameBuffer;
                pipelineDesc.Layout = {
                    { "POSITION", ShaderDataType::Float3 },
                    { "TEX_COORD", ShaderDataType::Float2 },
                };
                pipelineDesc.EnableBlend = false;
                pipelineDesc.EnableDepthTest = false;
                pipelineDesc.Wireframe = false;
                pipelineDesc.BackfaceCulling = true;

                ms_PipelineLibrary.Load<GraphicsPipeline>("FullscreenQuadPipeline", pipelineDesc);
            }
        }

        {
            FramebufferDescription fbDesc;
            fbDesc.SwapChainFrameBuffer = true;

            {
                GraphicsPipelineDescription pipelineDesc;
                pipelineDesc.Framebuffer = CreateRef<Framebuffer>(fbDesc, "ImGuiFramebuffer");
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
                GraphicsPipelineDescription pipelineDesc;
                pipelineDesc.Framebuffer = CreateRef<Framebuffer>(fbDesc, "SwapChainFramebuffer");
                pipelineDesc.Shader = ms_ShaderLibrary.Get<GraphicsShader>("FullscreenQuadShader");
                pipelineDesc.EnableBlend = false;
                pipelineDesc.EnableDepthTest = false;
                pipelineDesc.BackfaceCulling = true;
                pipelineDesc.Wireframe = false;
                pipelineDesc.Topology = Topology::Triangles;
                pipelineDesc.Layout = {
                    { "POSITION", ShaderDataType::Float3 },
                    { "TEX_COORD", ShaderDataType::Float2 },
                };

                ms_PipelineLibrary.Load<GraphicsPipeline>("SwapChainPipeline", pipelineDesc);
            }

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
        UploadBufferData(quadVertices, ms_FullscreenQuadVB.get());

        u16 quadIndices[] = { 0, 1, 2, 2, 3, 0 };

        BufferDescription ibDesc;
        ibDesc.ElementCount = _countof(quadIndices);
        ibDesc.ElementSize = sizeof(u16);
        ibDesc.IsDynamic = false;

        ms_FullscreenQuadIB = CreateRef<IndexBuffer>(ibDesc, IndexBufferFormat::U16, "FullscreenQuadIB(Renderer)");
        UploadBufferData(quadIndices, ms_FullscreenQuadIB.get());

        // Create error texture
        TextureDescription errorTextureDesc;
        errorTextureDesc.Width = 1;
        errorTextureDesc.Height = 1;
        errorTextureDesc.Format = TextureFormat::RGBA8;
        errorTextureDesc.MipLevels = 1;

        Vector<Vector<byte>> errorTextureData;
        errorTextureData.resize(errorTextureDesc.MipLevels);
        errorTextureData[0] = { 0xFF, 0x00, 0xFF, 0xFF }; // Just 1 pixel of pink color

        ms_ErrorTexture = CreateRef<Texture2D>(errorTextureDesc, errorTextureData, false, "ErrorTexture(Renderer)");

        // Create black texture and black texture cube
        TextureDescription blackTextureDesc;
        blackTextureDesc.Width = 1;
        blackTextureDesc.Height = 1;
        blackTextureDesc.Format = TextureFormat::RGBA8;
        blackTextureDesc.MipLevels = 1;

        Vector<Vector<byte>> blackTextureData;
        blackTextureData.resize(blackTextureDesc.MipLevels);
        blackTextureData[0] = { 0x00, 0x00, 0x00, 0xFF }; // Just 1 pixel of black color

        ms_BlackTexture = CreateRef<Texture2D>(blackTextureDesc, blackTextureData, false, "BlackTexture(Renderer)");

        Vector<Vector<byte>> blackTextureCubeData[6] = { 
            { blackTextureData }, 
            { blackTextureData }, 
            { blackTextureData }, 
            { blackTextureData }, 
            { blackTextureData }, 
            { blackTextureData } 
        };

        ms_BlackTextureCube = CreateRef<TextureCube>(blackTextureDesc, blackTextureCubeData, false, "BlackTextureCube(Renderer)");

        // Generate BRDF texture
        ms_BRDFTexture = Renderer::CreateBRDFTexture();

        // Create renderer materials
        ms_DefaultMaterial = CreateRef<Material>(ms_ShaderLibrary.Get<GraphicsShader>("MeshPBRShader"), MaterialFlags::DepthTested | MaterialFlags::TwoSided);
        ms_DefaultMaterial->SetUniform("AlbedoColor", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        ms_DefaultMaterial->SetUniform("Metalness", 0.5f);
        ms_DefaultMaterial->SetUniform("Roughness", 0.5f);

        // Create renderer materials
        ms_DefaultMaterialAnimated = CreateRef<Material>(ms_ShaderLibrary.Get<GraphicsShader>("MeshPBRAnimatedShader"), MaterialFlags::DepthTested | MaterialFlags::TwoSided);
        ms_DefaultMaterialAnimated->SetUniform("AlbedoColor", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        ms_DefaultMaterialAnimated->SetUniform("Metalness", 0.5f);
        ms_DefaultMaterialAnimated->SetUniform("Roughness", 0.5f);

        ms_ErrorMaterial = CreateRef<Material>(ms_ShaderLibrary.Get<GraphicsShader>("MeshPBRShader"), MaterialFlags::DepthTested | MaterialFlags::TwoSided);
        ms_ErrorMaterial->SetUniform("AlbedoColor", glm::vec4(1.0f, 0.0f, 1.0f, 1.0f));
        ms_ErrorMaterial->SetUniform("Metalness", 0.0f);
        ms_ErrorMaterial->SetUniform("Roughness", 1.0f);

        ms_ErrorMaterialAnimated = CreateRef<Material>(ms_ShaderLibrary.Get<GraphicsShader>("MeshPBRAnimatedShader"), MaterialFlags::DepthTested | MaterialFlags::TwoSided);
        ms_ErrorMaterialAnimated->SetUniform("AlbedoColor", glm::vec4(1.0f, 0.0f, 1.0f, 1.0f));
        ms_ErrorMaterialAnimated->SetUniform("Metalness", 0.0f);
        ms_ErrorMaterialAnimated->SetUniform("Roughness", 1.0f);

        // Wait for all copy/compute operations to complete before we continue
        Device::Get().GetCommandQueue(CommandQueueType::Compute)->Flush();
        Device::Get().GetCommandQueue(CommandQueueType::Copy)->Flush();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::Shutdown()
    {
        ms_PipelineLibrary.Clear();
        ms_ShaderLibrary.Clear();
        ms_FullscreenQuadVB.reset();
        ms_FullscreenQuadIB.reset();
        ms_BRDFTexture.reset();
        ms_ErrorTexture.reset();
        ms_BlackTexture.reset();
        ms_BlackTextureCube.reset();
        ms_DefaultMaterial.reset();
        ms_DefaultMaterialAnimated.reset();
        ms_ErrorMaterial.reset();
        ms_ErrorMaterialAnimated.reset();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::BeginFrame()
    {
        u32 currentFrameIndex = GetCurrentFrameIndex();

        Device::Get().ProcessDeferredReleases(currentFrameIndex);
        PIXBeginEvent(Device::Get().GetCommandQueue(CommandQueueType::Graphics)->GetD3DCommandQueue().Get(), 0, "Begin Frame");
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
    void Renderer::RenderMesh(Ref<CommandBuffer> commandBuffer, Ref<GraphicsPipeline> pipeline, Ref<Mesh> mesh, u32 submeshIdx, Ref<Material> overrideMaterial, Ref<ConstantBuffer> cameraCB, Ref<StructuredBuffer> animationSB, Ref<StructuredBuffer> lightsSB)
    {
        commandBuffer->SetGraphicsPipeline(pipeline.get());
        commandBuffer->SetVertexBuffer(mesh->GetVertexBuffer().get());
        commandBuffer->SetIndexBuffer(mesh->GetIndexBuffer().get());

        const Submesh& submesh = mesh->GetSubmeshes()[submeshIdx];
        Ref<Material> material = overrideMaterial ? overrideMaterial : mesh->GetMaterialTable()->GetMaterial(submesh.MaterialIndex);

        u32 currentRootParameter = 0;

        // Set root constants
        for (const auto& [bufferSlot, data] : material->GetUniformBuffersData())
        {
            commandBuffer->SetGraphicsRootConstants(currentRootParameter++, data.data(), data.size() / 4);
        }

        // Set constant buffers and structured buffers
        if (cameraCB)
        {
            commandBuffer->SetGraphicsConstantBuffer(currentRootParameter++, cameraCB.get());
        }

        if (lightsSB)
        {
            commandBuffer->SetGraphicsStructuredBuffer(currentRootParameter++, lightsSB.get());
        }

        if (animationSB)
        {
            commandBuffer->SetGraphicsStructuredBuffer(currentRootParameter++, animationSB.get());
        }

        // Transition textures
        for (auto& [_, texture] : material->GetTextures())
            if(texture)
                commandBuffer->TransitionResource(texture.get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

        // Set material descriptor tables
        if (material->IsDirty())
            material->UpdateDescriptorTables();

        commandBuffer->SetGraphicsDescriptorTable(currentRootParameter++, material->GetResourceDescriptorTable().GetBaseGpuDescriptor());
        commandBuffer->SetGraphicsDescriptorTable(currentRootParameter++, material->GetSamplerDescriptorTable().GetBaseGpuDescriptor());

        commandBuffer->DrawIndexed(submesh.IndexCount, 1, submesh.StartIndex, submesh.StartVertex, 0);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::RenderFullscreenQuad(Ref<CommandBuffer> commandBuffer, Ref<GraphicsPipeline> pipeline, Ref<ConstantBuffer> constantBuffer, Ref<Material> material)
    {
        commandBuffer->SetGraphicsPipeline(pipeline.get());
        commandBuffer->SetVertexBuffer(ms_FullscreenQuadVB.get());
        commandBuffer->SetIndexBuffer(ms_FullscreenQuadIB.get());

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

        // Transition textures
        for (auto& [_, texture] : material->GetTextures())
            if (texture)
                commandBuffer->TransitionResource(texture.get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

        // Set material descriptor tables
        if (material->IsDirty())
            material->UpdateDescriptorTables();

        commandBuffer->SetGraphicsDescriptorTable(currentRootParameter++, material->GetResourceDescriptorTable().GetBaseGpuDescriptor());
        commandBuffer->SetGraphicsDescriptorTable(currentRootParameter++, material->GetSamplerDescriptorTable().GetBaseGpuDescriptor());

        // Draw
        commandBuffer->DrawIndexed(ms_FullscreenQuadIB->GetElementCount(), 1, 0, 0, 0);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<TextureCube> Renderer::CreateEnvironmentMap(Ref<Texture2D> equirectTexture, u32 mapSize)
    {
        GPUDescriptorHeap* resourceHeap = Device::Get().GetGPUDescriptorHeap(DescriptorHeapType::ShaderResource);
        GPUDescriptorHeap* samplerHeap = Device::Get().GetGPUDescriptorHeap(DescriptorHeapType::Sampler);

        CommandQueue* computeQueue = Device::Get().GetCommandQueue(CommandQueueType::Compute);
        CommandQueue* copyQueue = Device::Get().GetCommandQueue(CommandQueueType::Copy);

        /////////////////////////////////////////////////////////////////////////////////////////////////
        //                        Phase 1: Equirectangular map to cubemap                              //
        /////////////////////////////////////////////////////////////////////////////////////////////////

        TextureDescription envMapDesc;
        envMapDesc.Width = mapSize;
        envMapDesc.Height = mapSize;
        envMapDesc.Format = equirectTexture->GetFormat();
        envMapDesc.MipLevels = (u32)glm::log2((f32)mapSize) + 1;
        envMapDesc.UsageFlags = equirectTexture->GetBindFlags();

        Vector<Vector<byte>> emptyCubeData[6];

        for (u32 face = 0; face < 6; face++)
            emptyCubeData[face].resize(envMapDesc.MipLevels);

        Ref<TextureCube> envMapUnfiltered = CreateRef<TextureCube>(envMapDesc, emptyCubeData, false, fmt::format("{}(Unfiltered)", equirectTexture->GetAssetFilepath().stem().string()).c_str());

        {
            PIXBeginEvent(computeQueue->GetD3DCommandQueue().Get(), 0, "EquirectToCubeMap");

            auto& d3dDevice = Device::Get().GetD3DDevice();

            DescriptorAllocation samplerTable = samplerHeap->AllocateTransient(1);
            Device::Get().CopyDescriptors(samplerTable, 1, &equirectTexture->GetSampler(), DescriptorHeapType::Sampler);

            Ref<CommandBuffer> computeCmdBuffer = computeQueue->GetCommandBuffer();
            computeCmdBuffer->Begin();
            computeCmdBuffer->TransitionResource(equirectTexture.get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            computeCmdBuffer->TransitionResource(envMapUnfiltered.get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            computeCmdBuffer->SetComputePipeline(ms_PipelineLibrary.Get<ComputePipeline>("EquirectToCubeMapPipeline").get());
            computeCmdBuffer->SetDescriptorHeaps(resourceHeap, samplerHeap);
            computeCmdBuffer->SetComputeDescriptorTable(2, samplerTable.GetBaseGpuDescriptor());

            for (u32 mip = 0; mip < envMapDesc.MipLevels; mip++)
            {
                D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptors[] = { equirectTexture->GetSRV(), envMapUnfiltered->GetArrayUAV(mip) };

                DescriptorAllocation resourceTable = resourceHeap->AllocateTransient(_countof(cpuDescriptors));
                Device::Get().CopyDescriptors(resourceTable, _countof(cpuDescriptors), cpuDescriptors, DescriptorHeapType::ShaderResource);

                computeCmdBuffer->SetComputeRootConstants(0, &mip, 1);
                computeCmdBuffer->SetComputeDescriptorTable(1, resourceTable.GetBaseGpuDescriptor());
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

        Ref<TextureCube> envMap = CreateRef<TextureCube>(envMapDesc, emptyCubeData, equirectTexture->IsReadable(), fmt::format("{}", equirectTexture->GetAssetFilepath().stem().string()).c_str());

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

            DescriptorAllocation samplerTable = samplerHeap->AllocateTransient(1);
            Device::Get().CopyDescriptors(samplerTable, 1, &envMapUnfiltered->GetSampler(), DescriptorHeapType::Sampler);

            Ref<CommandBuffer> computeCmdBuffer = computeQueue->GetCommandBuffer();
            computeCmdBuffer->Begin();
            computeCmdBuffer->TransitionResource(envMapUnfiltered.get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            computeCmdBuffer->TransitionResource(envMap.get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            computeCmdBuffer->SetComputePipeline(ms_PipelineLibrary.Get<ComputePipeline>("CubeMapPrefilterPipeline").get());
            computeCmdBuffer->SetDescriptorHeaps(resourceHeap, samplerHeap);
            computeCmdBuffer->SetComputeDescriptorTable(2, samplerTable.GetBaseGpuDescriptor());

            u32 width = glm::max(envMapDesc.Width / 2, 1u);
            u32 height = glm::max(envMapDesc.Height / 2, 1u);

            for (u32 mip = 1; mip < envMapDesc.MipLevels; mip++)
            {
                D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptors[] = { envMapUnfiltered->GetSRV(), envMap->GetArrayUAV(mip) };

                DescriptorAllocation resourceTable = resourceHeap->AllocateTransient(_countof(cpuDescriptors));
                Device::Get().CopyDescriptors(resourceTable, _countof(cpuDescriptors), cpuDescriptors, DescriptorHeapType::ShaderResource);

                f32 roughness = mip / glm::max(envMapDesc.MipLevels - 1.0f, 1.0f);
                computeCmdBuffer->SetComputeRootConstants(0, &roughness, 1);
                computeCmdBuffer->SetComputeDescriptorTable(1, resourceTable.GetBaseGpuDescriptor());
                computeCmdBuffer->Dispatch(glm::max(width / 32, 1u), glm::max(height / 32, 1u), 6);

                width = glm::max(width / 2, 1u);
                height = glm::max(height / 2, 1u);
            }

            computeCmdBuffer->TransitionResource(envMap.get(), D3D12_RESOURCE_STATE_COMMON);
            computeCmdBuffer->End();
            computeQueue->WaitForQueue(copyQueue);
            computeQueue->ExecuteCommandList(computeCmdBuffer);

            PIXEndEvent(computeQueue->GetD3DCommandQueue().Get());
        }

        // Wait for all compute operations to complete
        computeQueue->Flush();

        if (envMap->IsReadable())
        {
            // If the cubemap is readable, get generated data from the GPU and copy it to the CPU
            for (u32 face = 0; face < 6; face++)
            {
                for (u32 mip = 0; mip < envMap->GetMipLevels(); mip++)
                {
                    Ref<ReadbackBuffer> buffer = Renderer::ReadbackTextureData(envMap.get(), mip, face);

                    Vector<byte> pixelData(buffer->GetSize(), 0);
                    void* mappedData = buffer->Map(0, 0);
                    memcpy(pixelData.data(), mappedData, pixelData.size());
                    buffer->Unmap();

                    envMap->SetPixels(pixelData, face, mip);
                }
            }
        }

        return envMap;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<TextureCube> Renderer::CreateIrradianceMap(Ref<TextureCube> environmentMap, u32 mapSize)
    {
        GPUDescriptorHeap* resourceHeap = Device::Get().GetGPUDescriptorHeap(DescriptorHeapType::ShaderResource);
        GPUDescriptorHeap* samplerHeap = Device::Get().GetGPUDescriptorHeap(DescriptorHeapType::Sampler);

        CommandQueue* computeQueue = Device::Get().GetCommandQueue(CommandQueueType::Compute);
        CommandQueue* copyQueue = Device::Get().GetCommandQueue(CommandQueueType::Copy);
        CommandQueue* gfxQueue = Device::Get().GetCommandQueue(CommandQueueType::Graphics);

        Ref<CommandBuffer> gfxCmdBuffer = gfxQueue->GetCommandBuffer();
        gfxCmdBuffer->Begin();
        gfxCmdBuffer->TransitionResource(environmentMap.get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        gfxCmdBuffer->End();
        gfxQueue->ExecuteCommandList(gfxCmdBuffer);

        TextureDescription irradianceMapDesc;
        irradianceMapDesc.Width = mapSize;
        irradianceMapDesc.Height = mapSize;
        irradianceMapDesc.Format = TextureFormat::RGBA16F;
        irradianceMapDesc.MipLevels = 1;
        irradianceMapDesc.UsageFlags = TextureBindFlags::UnorderedAccess;

        Vector<Vector<byte>> emptyCubeData[6];

        for (u32 face = 0; face < 6; face++)
            emptyCubeData[face].resize(irradianceMapDesc.MipLevels);

        Ref<TextureCube> irradianceMap = CreateRef<TextureCube>(irradianceMapDesc, emptyCubeData, false, fmt::format("{}(IrradianceMap)", environmentMap->GetAssetFilepath().stem().string()).c_str());

        D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptors[] = { environmentMap->GetSRV(), irradianceMap->GetArrayUAV() };

        DescriptorAllocation resourceTable = resourceHeap->AllocateTransient(_countof(cpuDescriptors));
        Device::Get().CopyDescriptors(resourceTable, _countof(cpuDescriptors), cpuDescriptors, DescriptorHeapType::ShaderResource);

        DescriptorAllocation samplerTable = samplerHeap->AllocateTransient(1);
        Device::Get().CopyDescriptors(samplerTable, 1, &environmentMap->GetSampler(), DescriptorHeapType::Sampler);

        PIXBeginEvent(computeQueue->GetD3DCommandQueue().Get(), 0, "GenerateIrradianceMap");

        Ref<CommandBuffer> computeCmdBuffer = computeQueue->GetCommandBuffer();
        computeCmdBuffer->Begin();
        computeCmdBuffer->TransitionResource(irradianceMap.get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        computeCmdBuffer->SetComputePipeline(ms_PipelineLibrary.Get<ComputePipeline>("CubeMapIrradiancePipeline").get());
        computeCmdBuffer->SetDescriptorHeaps(resourceHeap, samplerHeap);
        computeCmdBuffer->SetComputeDescriptorTable(0, resourceTable.GetBaseGpuDescriptor());
        computeCmdBuffer->SetComputeDescriptorTable(1, samplerTable.GetBaseGpuDescriptor());
        computeCmdBuffer->Dispatch(glm::max(irradianceMapDesc.Width / 32, 1u), glm::max(irradianceMapDesc.Height / 32, 1u), 6);
        computeCmdBuffer->TransitionResource(environmentMap.get(), D3D12_RESOURCE_STATE_COMMON);
        computeCmdBuffer->TransitionResource(irradianceMap.get(), D3D12_RESOURCE_STATE_COMMON);
        computeCmdBuffer->End();
        computeQueue->ExecuteCommandList(computeCmdBuffer);

        PIXEndEvent(computeQueue->GetD3DCommandQueue().Get());

        // Wait for all compute operations to complete
        computeQueue->Flush();

        return irradianceMap;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<Texture2D> Renderer::CreateBRDFTexture()
    {
        TextureDescription brdfDesc;
        brdfDesc.Width = 256;
        brdfDesc.Height = 256;
        brdfDesc.Format = TextureFormat::RG16F;
        brdfDesc.MipLevels = 1;
        brdfDesc.UsageFlags = TextureBindFlags::UnorderedAccess;
        brdfDesc.Wrap = TextureWrap::Clamp;

        Vector<Vector<byte>> emptyData;
        emptyData.resize(brdfDesc.MipLevels);

        Ref<Texture2D> brdfTexture = CreateRef<Texture2D>(brdfDesc, emptyData, false, "BRDFTexture(Renderer)");

        GPUDescriptorHeap* resourceHeap = Device::Get().GetGPUDescriptorHeap(DescriptorHeapType::ShaderResource);
        DescriptorAllocation resourceTable = resourceHeap->AllocateTransient(1);
        Device::Get().CopyDescriptors(resourceTable, 1, &brdfTexture->GetUAV(), DescriptorHeapType::ShaderResource);

        CommandQueue* computeQueue = Device::Get().GetCommandQueue(CommandQueueType::Compute);
        Ref<CommandBuffer> computeCmdBuffer = computeQueue->GetCommandBuffer();
        computeCmdBuffer->Begin();
        computeCmdBuffer->TransitionResource(brdfTexture.get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        computeCmdBuffer->SetComputePipeline(ms_PipelineLibrary.Get<ComputePipeline>("BRDFPipeline").get());
        computeCmdBuffer->SetDescriptorHeaps(resourceHeap, nullptr);
        computeCmdBuffer->SetComputeDescriptorTable(0, resourceTable.GetBaseGpuDescriptor());
        computeCmdBuffer->Dispatch(glm::max(brdfDesc.Width / 32, 1u), glm::max(brdfDesc.Height / 32, 1u), 1);
        computeCmdBuffer->TransitionResource(brdfTexture.get(), D3D12_RESOURCE_STATE_COMMON);
        computeCmdBuffer->End();
        computeQueue->ExecuteCommandList(computeCmdBuffer);

        return brdfTexture;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::GenerateMips(Ref<Texture2D> texture)
    {
        CommandQueue* computeQueue = Device::Get().GetCommandQueue(CommandQueueType::Compute);
        PIXBeginEvent(computeQueue->GetD3DCommandQueue().Get(), 0, "GenerateMips");

        GPUDescriptorHeap* resourceHeap = Device::Get().GetGPUDescriptorHeap(DescriptorHeapType::ShaderResource);
        GPUDescriptorHeap* samplerHeap = Device::Get().GetGPUDescriptorHeap(DescriptorHeapType::Sampler);

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

        DescriptorAllocation samplerTable = samplerHeap->AllocateTransient(1);
        Device::Get().GetD3DDevice()->CreateSampler(&samplerDesc, samplerTable.GetBaseCpuDescriptor());

        // Run compute shader for each mip
        Ref<CommandBuffer> computeCmdBuffer = computeQueue->GetCommandBuffer();
        computeCmdBuffer->Begin();
        computeCmdBuffer->SetComputePipeline(Renderer::GetPipelineLibrary().Get<ComputePipeline>("GenerateMipsPipeline").get());
        computeCmdBuffer->SetDescriptorHeaps(resourceHeap, samplerHeap);
        computeCmdBuffer->SetComputeDescriptorTable(2, samplerTable.GetBaseGpuDescriptor());

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

            DescriptorAllocation resourceTable = resourceHeap->AllocateTransient(_countof(cpuDescriptors));
            Device::Get().CopyDescriptors(resourceTable, _countof(cpuDescriptors), cpuDescriptors, DescriptorHeapType::ShaderResource);

            GenerateMipCB constants;
            constants.TexelSize = { 1.0f / width, 1.0f / height };
            constants.TopMipLevel = mip - 1;

            computeCmdBuffer->TransitionResource(texture.get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, mip - 1);
            computeCmdBuffer->TransitionResource(texture.get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, mip);
            computeCmdBuffer->SetComputeRootConstants(0, &constants, 3);
            computeCmdBuffer->SetComputeDescriptorTable(1, resourceTable.GetBaseGpuDescriptor());
            computeCmdBuffer->Dispatch(glm::max(width / 8, 1u), glm::max(height / 8, 1u), 1);

            computeCmdBuffer->AddUAVBarrier(texture.get());

            width = glm::max(width / 2, 1u);
            height = glm::max(height / 2, 1u);
        }

        for(u32 mip = 0; mip < texture->GetMipLevels(); mip++)
            computeCmdBuffer->TransitionResource(texture.get(), D3D12_RESOURCE_STATE_COMMON, mip);

        computeCmdBuffer->End();

        // Wait for any copy operations to complate in order to make sure we have all the data already uploaded before executing
        computeQueue->WaitForQueue(Device::Get().GetCommandQueue(CommandQueueType::Copy));
        computeQueue->ExecuteCommandList(computeCmdBuffer);
        computeQueue->Flush();

        if (texture->IsReadable())
        {
            // If the texture is readable, get generated data from the GPU and copy it to the CPU
            for (u32 mip = 1; mip < texture->GetMipLevels(); mip++)
            {
                Ref<ReadbackBuffer> buffer = Renderer::ReadbackTextureData(texture.get(), mip);

                Vector<byte> pixelData(buffer->GetSize(), 0);
                void* mappedData = buffer->Map(0, 0);
                memcpy(pixelData.data(), mappedData, pixelData.size());
                buffer->Unmap();

                texture->SetPixels(pixelData, mip);
            }
        }

        PIXEndEvent(computeQueue->GetD3DCommandQueue().Get());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::UploadBufferData(const void* srcData, const Buffer* buffer)
    {
        CommandQueue* copyQueue = Device::Get().GetCommandQueue(CommandQueueType::Copy);
        Ref<CommandBuffer> copyCommandBuffer = copyQueue->GetCommandBuffer();
        copyCommandBuffer->Begin();
        copyCommandBuffer->UploadBufferData(srcData, buffer);
        copyCommandBuffer->End();
        copyQueue->ExecuteCommandList(copyCommandBuffer);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::UploadTextureData(const void* srcData, const Texture* texture, u32 mip, u32 slice)
    {
        CommandQueue* copyQueue = Device::Get().GetCommandQueue(CommandQueueType::Copy);
        Ref<CommandBuffer> copyCommandBuffer = copyQueue->GetCommandBuffer();
        copyCommandBuffer->Begin();
        copyCommandBuffer->UploadTextureData(srcData, texture, mip, slice);
        copyCommandBuffer->End();
        copyQueue->ExecuteCommandList(copyCommandBuffer);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<ReadbackBuffer> Renderer::ReadbackTextureData(const Texture* texture, u32 mip, u32 slice)
    {
        CommandQueue* copyQueue = Device::Get().GetCommandQueue(CommandQueueType::Copy);
        Ref<CommandBuffer> copyCommandBuffer = copyQueue->GetCommandBuffer();
        copyCommandBuffer->Begin();
        Ref<ReadbackBuffer> readbackBuffer = copyCommandBuffer->ReadbackTextureData(texture, mip, slice);
        copyCommandBuffer->End();
        copyQueue->ExecuteCommandList(copyCommandBuffer);
        copyQueue->Flush();
        return readbackBuffer;
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

    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<Material> Renderer::GetDefaultMaterial()
    {
        return ms_DefaultMaterial;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<Material> Renderer::GetDefaultMaterialAnimated()
    {
        return ms_DefaultMaterialAnimated;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<Material> Renderer::GetErrorMaterial()
    {
        return ms_ErrorMaterial;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<Material> Renderer::GetErrorMaterialAnimated()
    {
        return ms_ErrorMaterialAnimated;
    }
}