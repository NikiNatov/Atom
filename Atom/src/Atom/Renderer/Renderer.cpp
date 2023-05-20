#include "atompch.h"
#include "Renderer.h"

#include "Atom/Core/Application.h"
#include "Atom/Renderer/CommandBuffer.h"
#include "Atom/Renderer/CommandQueue.h"
#include "Atom/Renderer/Texture.h"
#include "Atom/Renderer/Material.h"
#include "Atom/Renderer/Framebuffer.h"
#include "Atom/Renderer/Buffer.h"
#include "Atom/Renderer/LightEnvironment.h"
#include "Atom/Asset/MeshAsset.h"

#include <pix3.h>

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::Initialize(const RendererConfig& config)
    {
        ms_Config = config;
        
        // Create samplers
        ms_Samplers.reserve(u32(TextureFilter::NumFilters) * u32(TextureWrap::NumWraps));
        for (u32 i = 0; i < u32(TextureFilter::NumFilters); i++)
        {
            for (u32 j = 0; j < u32(TextureWrap::NumWraps); j++)
            {
                ms_Samplers.push_back(CreateRef<TextureSampler>((TextureFilter)i, (TextureWrap)j));
            }
        }

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
            fbDesc.Attachments[AttachmentPoint::Color0] = { TextureFormat::RGBA16F, ClearValue(0.2f, 0.2f, 0.2f, 1.0f), TextureFilter::Linear, TextureWrap::Clamp };
            fbDesc.Attachments[AttachmentPoint::Depth] = { TextureFormat::Depth24Stencil8, ClearValue(1.0f, 0), TextureFilter::Linear, TextureWrap::Clamp };

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
            fbDesc.Attachments[AttachmentPoint::Color0] = { TextureFormat::RGBA8, ClearValue(0.2f, 0.2f, 0.2f, 1.0f), TextureFilter::Linear, TextureWrap::Clamp };

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
        {
            TextureDescription errorTextureDesc;
            errorTextureDesc.Width = 1;
            errorTextureDesc.Height = 1;
            errorTextureDesc.Format = TextureFormat::RGBA8;
            errorTextureDesc.MipLevels = 1;

            ms_ErrorTexture = CreateRef<Texture>(errorTextureDesc, "ErrorTexture(Renderer)");

            const byte errorTextureData[] = { 0xFF, 0x00, 0xFF, 0xFF };
            UploadTextureData(errorTextureData, ms_ErrorTexture);
        }

        // Create black texture and black texture cube
        {
            TextureDescription blackTextureDesc;
            blackTextureDesc.Width = 1;
            blackTextureDesc.Height = 1;
            blackTextureDesc.Format = TextureFormat::RGBA8;
            blackTextureDesc.MipLevels = 1;

            ms_BlackTexture = CreateRef<Texture>(blackTextureDesc, "BlackTexture(Renderer)");

            blackTextureDesc.ArraySize = 6;
            blackTextureDesc.Flags |= TextureFlags::CubeMap;

            ms_BlackTextureCube = CreateRef<Texture>(blackTextureDesc, "BlackTextureCube(Renderer)");

            const byte blackTextureData[] = { 0x00, 0x00, 0x00, 0xFF }; // Just 1 pixel of black color
            UploadTextureData(blackTextureData, ms_BlackTexture);

            for(u32 face = 0; face < 6; face++)
                UploadTextureData(blackTextureData, ms_BlackTextureCube, 0, face);
        }

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
        ms_Samplers.clear();
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
    void Renderer::RenderMesh(Ref<CommandBuffer> commandBuffer, Ref<Mesh> mesh, u32 submeshIdx, Ref<Material> overrideMaterial)
    {
        const Submesh& submesh = mesh->GetSubmeshes()[submeshIdx];
        Ref<Material> material = overrideMaterial ? overrideMaterial : mesh->GetMaterialTable()->GetMaterial(submesh.MaterialIndex)->GetResource();

        // Transition textures
        for (auto& [_, texture] : material->GetTextures())
            if(texture)
                commandBuffer->TransitionResource(texture.get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

        // Set material descriptor tables
        if (material->IsDirty())
            material->UpdateDescriptorTables();

        commandBuffer->SetGraphicsConstants(ShaderBindPoint::Material, material->GetConstantsData().data(), material->GetConstantsData().size() / 4);
        commandBuffer->SetGraphicsDescriptorTables(ShaderBindPoint::Material, material->GetResourceTable().GetBaseGpuDescriptor(), material->GetSamplerTable().GetBaseGpuDescriptor());
        commandBuffer->SetVertexBuffer(mesh->GetVertexBuffer().get());
        commandBuffer->SetIndexBuffer(mesh->GetIndexBuffer().get());
        commandBuffer->DrawIndexed(submesh.IndexCount, 1, submesh.StartIndex, submesh.StartVertex, 0);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::RenderFullscreenQuad(Ref<CommandBuffer> commandBuffer, Ref<Texture> texture)
    {
        if (texture)
        {
            DescriptorAllocation resourceTable = Device::Get().GetGPUDescriptorHeap(DescriptorHeapType::ShaderResource)->AllocateTransient(1);
            Device::Get().CopyDescriptors(resourceTable, 1, &texture->GetSRV()->GetDescriptor(), DescriptorHeapType::ShaderResource);

            DescriptorAllocation samplerTable = Device::Get().GetGPUDescriptorHeap(DescriptorHeapType::Sampler)->AllocateTransient(1);
            Device::Get().CopyDescriptors(samplerTable, 1, &Renderer::GetSampler(TextureFilter::Linear, TextureWrap::Clamp)->GetDescriptor(), DescriptorHeapType::Sampler);

            commandBuffer->TransitionResource(texture.get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            commandBuffer->SetGraphicsDescriptorTables(ShaderBindPoint::Instance, resourceTable.GetBaseGpuDescriptor(), samplerTable.GetBaseGpuDescriptor());
        }

        commandBuffer->SetVertexBuffer(ms_FullscreenQuadVB.get());
        commandBuffer->SetIndexBuffer(ms_FullscreenQuadIB.get());
        commandBuffer->DrawIndexed(ms_FullscreenQuadIB->GetElementCount(), 1, 0, 0, 0);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<Texture> Renderer::CreateEnvironmentMap(Ref<Texture> equirectTexture, u32 mapSize, const char* debugName)
    {
        GPUDescriptorHeap* resourceHeap = Device::Get().GetGPUDescriptorHeap(DescriptorHeapType::ShaderResource);
        GPUDescriptorHeap* samplerHeap = Device::Get().GetGPUDescriptorHeap(DescriptorHeapType::Sampler);

        CommandQueue* computeQueue = Device::Get().GetCommandQueue(CommandQueueType::Compute);
        CommandQueue* copyQueue = Device::Get().GetCommandQueue(CommandQueueType::Copy);

        /////////////////////////////////////////////////////////////////////////////////////////////////
        //                        Phase 1: Equirectangular map to cubemap                              //
        /////////////////////////////////////////////////////////////////////////////////////////////////

        TextureDescription envMapDesc;
        envMapDesc.Format = equirectTexture->GetFormat();
        envMapDesc.Width = mapSize;
        envMapDesc.Height = mapSize;
        envMapDesc.ArraySize = 6;
        envMapDesc.MipLevels = (u32)glm::log2((f32)mapSize) + 1;
        envMapDesc.Flags = TextureFlags::ShaderResource | TextureFlags::UnorderedAccess | TextureFlags::CubeMap;

        Ref<Texture> envMapUnfiltered = CreateRef<Texture>(envMapDesc, fmt::format("{}(Unfiltered)", debugName).c_str());

        {
            PIXBeginEvent(computeQueue->GetD3DCommandQueue().Get(), 0, "EquirectToCubeMap");

            auto& d3dDevice = Device::Get().GetD3DDevice();

            DescriptorAllocation samplerTable = samplerHeap->AllocateTransient(1);
            Device::Get().CopyDescriptors(samplerTable, 1, &Renderer::GetSampler(TextureFilter::Linear, TextureWrap::Repeat)->GetDescriptor(), DescriptorHeapType::Sampler);

            Ref<CommandBuffer> computeCmdBuffer = computeQueue->GetCommandBuffer();
            computeCmdBuffer->Begin();
            computeCmdBuffer->TransitionResource(equirectTexture.get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            computeCmdBuffer->TransitionResource(envMapUnfiltered.get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            computeCmdBuffer->SetComputePipeline(ms_PipelineLibrary.Get<ComputePipeline>("EquirectToCubeMapPipeline").get());
            computeCmdBuffer->SetDescriptorHeaps(resourceHeap, samplerHeap);

            for (u32 mip = 0; mip < envMapDesc.MipLevels; mip++)
            {
                Ref<Texture> mipView = CreateRef<Texture>(*envMapUnfiltered, mip, UINT32_MAX, fmt::format("{}(Unfiltered)_Mip{}", debugName, mip).c_str());
                D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptors[] = { equirectTexture->GetSRV()->GetDescriptor(), mipView->GetUAV()->GetDescriptor() };

                DescriptorAllocation resourceTable = resourceHeap->AllocateTransient(_countof(cpuDescriptors));
                Device::Get().CopyDescriptors(resourceTable, _countof(cpuDescriptors), cpuDescriptors, DescriptorHeapType::ShaderResource);

                computeCmdBuffer->SetComputeConstants(ShaderBindPoint::Instance, &mip, 1);
                computeCmdBuffer->SetComputeDescriptorTables(ShaderBindPoint::Instance, resourceTable.GetBaseGpuDescriptor(), samplerTable.GetBaseGpuDescriptor());
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

        Ref<Texture> envMap = CreateRef<Texture>(envMapDesc, fmt::format("{}", debugName).c_str());

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
            Device::Get().CopyDescriptors(samplerTable, 1, &Renderer::GetSampler(TextureFilter::Linear, TextureWrap::Repeat)->GetDescriptor(), DescriptorHeapType::Sampler);

            Ref<CommandBuffer> computeCmdBuffer = computeQueue->GetCommandBuffer();
            computeCmdBuffer->Begin();
            computeCmdBuffer->TransitionResource(envMapUnfiltered.get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            computeCmdBuffer->TransitionResource(envMap.get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            computeCmdBuffer->SetComputePipeline(ms_PipelineLibrary.Get<ComputePipeline>("CubeMapPrefilterPipeline").get());
            computeCmdBuffer->SetDescriptorHeaps(resourceHeap, samplerHeap);

            u32 width = glm::max(envMapDesc.Width / 2, 1u);
            u32 height = glm::max(envMapDesc.Height / 2, 1u);

            for (u32 mip = 1; mip < envMapDesc.MipLevels; mip++)
            {
                Ref<Texture> mipView = CreateRef<Texture>(*envMap, mip, UINT32_MAX, fmt::format("{}_Mip{}", debugName, mip).c_str());
                D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptors[] = { envMapUnfiltered->GetSRV()->GetDescriptor(), mipView->GetUAV()->GetDescriptor() };

                DescriptorAllocation resourceTable = resourceHeap->AllocateTransient(_countof(cpuDescriptors));
                Device::Get().CopyDescriptors(resourceTable, _countof(cpuDescriptors), cpuDescriptors, DescriptorHeapType::ShaderResource);

                f32 roughness = mip / glm::max(envMapDesc.MipLevels - 1.0f, 1.0f);
                computeCmdBuffer->SetComputeConstants(ShaderBindPoint::Instance, &roughness, 1);
                computeCmdBuffer->SetComputeDescriptorTables(ShaderBindPoint::Instance, resourceTable.GetBaseGpuDescriptor(), samplerTable.GetBaseGpuDescriptor());
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

        return envMap;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<Texture> Renderer::CreateIrradianceMap(Ref<Texture> environmentMap, u32 mapSize, const char* debugName)
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
        irradianceMapDesc.Format = TextureFormat::RGBA16F;
        irradianceMapDesc.Width = mapSize;
        irradianceMapDesc.Height = mapSize;
        irradianceMapDesc.ArraySize = 6;
        irradianceMapDesc.MipLevels = 1;
        irradianceMapDesc.Flags = TextureFlags::UnorderedAccess | TextureFlags::ShaderResource | TextureFlags::CubeMap;

        Ref<Texture> irradianceMap = CreateRef<Texture>(irradianceMapDesc, fmt::format("{}(IrradianceMap)", debugName).c_str());

        D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptors[] = { environmentMap->GetSRV()->GetDescriptor(), irradianceMap->GetUAV()->GetDescriptor() };

        DescriptorAllocation resourceTable = resourceHeap->AllocateTransient(_countof(cpuDescriptors));
        Device::Get().CopyDescriptors(resourceTable, _countof(cpuDescriptors), cpuDescriptors, DescriptorHeapType::ShaderResource);

        DescriptorAllocation samplerTable = samplerHeap->AllocateTransient(1);
        Device::Get().CopyDescriptors(samplerTable, 1, &Renderer::GetSampler(TextureFilter::Linear, TextureWrap::Repeat)->GetDescriptor(), DescriptorHeapType::Sampler);

        PIXBeginEvent(computeQueue->GetD3DCommandQueue().Get(), 0, "GenerateIrradianceMap");

        Ref<CommandBuffer> computeCmdBuffer = computeQueue->GetCommandBuffer();
        computeCmdBuffer->Begin();
        computeCmdBuffer->TransitionResource(irradianceMap.get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        computeCmdBuffer->SetComputePipeline(ms_PipelineLibrary.Get<ComputePipeline>("CubeMapIrradiancePipeline").get());
        computeCmdBuffer->SetDescriptorHeaps(resourceHeap, samplerHeap);
        computeCmdBuffer->SetComputeDescriptorTables(ShaderBindPoint::Instance, resourceTable.GetBaseGpuDescriptor(), samplerTable.GetBaseGpuDescriptor());
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
    Ref<Texture> Renderer::CreateBRDFTexture()
    {
        TextureDescription brdfDesc;
        brdfDesc.Width = 256;
        brdfDesc.Height = 256;
        brdfDesc.Format = TextureFormat::RG16F;
        brdfDesc.MipLevels = 1;
        brdfDesc.Flags = TextureFlags::UnorderedAccess | TextureFlags::ShaderResource;

        Ref<Texture> brdfTexture = CreateRef<Texture>(brdfDesc, "BRDFTexture(Renderer)");

        GPUDescriptorHeap* resourceHeap = Device::Get().GetGPUDescriptorHeap(DescriptorHeapType::ShaderResource);
        DescriptorAllocation resourceTable = resourceHeap->AllocateTransient(1);
        Device::Get().CopyDescriptors(resourceTable, 1, &brdfTexture->GetUAV()->GetDescriptor(), DescriptorHeapType::ShaderResource);

        CommandQueue* computeQueue = Device::Get().GetCommandQueue(CommandQueueType::Compute);
        Ref<CommandBuffer> computeCmdBuffer = computeQueue->GetCommandBuffer();
        computeCmdBuffer->Begin();
        computeCmdBuffer->TransitionResource(brdfTexture.get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        computeCmdBuffer->SetComputePipeline(ms_PipelineLibrary.Get<ComputePipeline>("BRDFPipeline").get());
        computeCmdBuffer->SetDescriptorHeaps(resourceHeap, nullptr);
        computeCmdBuffer->SetComputeDescriptorTables(ShaderBindPoint::Instance, resourceTable.GetBaseGpuDescriptor());
        computeCmdBuffer->Dispatch(glm::max(brdfDesc.Width / 32, 1u), glm::max(brdfDesc.Height / 32, 1u), 1);
        computeCmdBuffer->TransitionResource(brdfTexture.get(), D3D12_RESOURCE_STATE_COMMON);
        computeCmdBuffer->End();
        computeQueue->ExecuteCommandList(computeCmdBuffer);

        return brdfTexture;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::GenerateMips(Ref<Texture> texture)
    {
        CommandQueue* computeQueue = Device::Get().GetCommandQueue(CommandQueueType::Compute);
        PIXBeginEvent(computeQueue->GetD3DCommandQueue().Get(), 0, "GenerateMips");

        GPUDescriptorHeap* resourceHeap = Device::Get().GetGPUDescriptorHeap(DescriptorHeapType::ShaderResource);
        GPUDescriptorHeap* samplerHeap = Device::Get().GetGPUDescriptorHeap(DescriptorHeapType::Sampler);

        // Run compute shader for each mip
        Ref<CommandBuffer> computeCmdBuffer = computeQueue->GetCommandBuffer();
        computeCmdBuffer->Begin();
        computeCmdBuffer->SetComputePipeline(Renderer::GetPipelineLibrary().Get<ComputePipeline>("GenerateMipsPipeline").get());
        computeCmdBuffer->SetDescriptorHeaps(resourceHeap, samplerHeap);

        u32 width = glm::max(texture->GetWidth() / 2, 1u);
        u32 height = glm::max(texture->GetHeight() / 2, 1u);

        struct GenerateMipCB
        {
            glm::vec2 TexelSize;
            u32 TopMipLevel;
        };

        DescriptorAllocation samplerTable = samplerHeap->AllocateTransient(1);
        Device::Get().CopyDescriptors(samplerTable, 1, &Renderer::GetSampler(TextureFilter::Linear, TextureWrap::Clamp)->GetDescriptor(), DescriptorHeapType::Sampler);

        for (u32 mip = 1; mip < texture->GetMipLevels(); mip++)
        {
            Ref<Texture> mipView = CreateRef<Texture>(*texture, mip, UINT32_MAX, "MipView");
            D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptors[] = { texture->GetSRV()->GetDescriptor(), mipView->GetUAV()->GetDescriptor() };

            DescriptorAllocation resourceTable = resourceHeap->AllocateTransient(_countof(cpuDescriptors));
            Device::Get().CopyDescriptors(resourceTable, _countof(cpuDescriptors), cpuDescriptors, DescriptorHeapType::ShaderResource);

            GenerateMipCB constants;
            constants.TexelSize = { 1.0f / width, 1.0f / height };
            constants.TopMipLevel = mip - 1;

            computeCmdBuffer->TransitionResource(texture.get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, mip - 1);
            computeCmdBuffer->TransitionResource(texture.get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, mip);
            computeCmdBuffer->SetComputeConstants(ShaderBindPoint::Instance, &constants, 3);
            computeCmdBuffer->SetComputeDescriptorTables(ShaderBindPoint::Instance, resourceTable.GetBaseGpuDescriptor(), samplerTable.GetBaseGpuDescriptor());
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
    void Renderer::UploadTextureData(const void* srcData, Ref<Texture> texture, u32 mip, u32 slice)
    {
        CommandQueue* copyQueue = Device::Get().GetCommandQueue(CommandQueueType::Copy);
        Ref<CommandBuffer> copyCommandBuffer = copyQueue->GetCommandBuffer();
        copyCommandBuffer->Begin();
        copyCommandBuffer->UploadTextureData(srcData, texture.get(), mip, slice);
        copyCommandBuffer->End();
        copyQueue->ExecuteCommandList(copyCommandBuffer);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<ReadbackBuffer> Renderer::ReadbackTextureData(Ref<Texture> texture, u32 mip, u32 slice)
    {
        CommandQueue* copyQueue = Device::Get().GetCommandQueue(CommandQueueType::Copy);
        Ref<CommandBuffer> copyCommandBuffer = copyQueue->GetCommandBuffer();
        copyCommandBuffer->Begin();
        Ref<ReadbackBuffer> readbackBuffer = copyCommandBuffer->ReadbackTextureData(texture.get(), mip, slice);
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
    Ref<Texture> Renderer::GetBRDF()
    {
        return ms_BRDFTexture;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<Texture> Renderer::GetErrorTexture()
    {
        return ms_ErrorTexture;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<Texture> Renderer::GetBlackTexture()
    {
        return ms_BlackTexture;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<Texture> Renderer::GetBlackTextureCube()
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

    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<TextureSampler> Renderer::GetSampler(TextureFilter filter, TextureWrap wrap)
    {
        return ms_Samplers[u32(filter) * u32(TextureFilter::NumFilters) + u32(wrap)];
    }
}