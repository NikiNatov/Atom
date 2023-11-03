#include "atompch.h"
#include "Renderer.h"

#include "Atom/Core/Application.h"
#include "Atom/Core/DirectX12/DirectX12Utils.h"
#include "Atom/Renderer/CommandQueue.h"
#include "Atom/Renderer/EngineResources.h"
#include "Atom/Asset/MeshAsset.h"

#include <pix3.h>

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::Initialize(const RendererConfig& config)
    {
        ms_Config = config;

        // Compile and load shaders
        ShaderCompiler::SetOutputDirectory("../Atom/shaders/bin");
        ms_ShaderLibrary.LoadGraphicsShader("../Atom/shaders/MeshPBRShader.hlsl");
        ms_ShaderLibrary.LoadGraphicsShader("../Atom/shaders/MeshPBRAnimatedShader.hlsl");
        ms_ShaderLibrary.LoadGraphicsShader("../Atom/shaders/SkyBoxShader.hlsl");
        ms_ShaderLibrary.LoadGraphicsShader("../Atom/shaders/ImGuiShader.hlsl");
        ms_ShaderLibrary.LoadGraphicsShader("../Atom/shaders/CompositeShader.hlsl");
        ms_ShaderLibrary.LoadGraphicsShader("../Atom/shaders/FullscreenQuadShader.hlsl");
        ms_ShaderLibrary.LoadComputeShader("../Atom/shaders/GenerateMips.hlsl");
        ms_ShaderLibrary.LoadComputeShader("../Atom/shaders/EquirectToCubeMap.hlsl");
        ms_ShaderLibrary.LoadComputeShader("../Atom/shaders/CubeMapPrefilter.hlsl");
        ms_ShaderLibrary.LoadComputeShader("../Atom/shaders/CubeMapIrradiance.hlsl");
        ms_ShaderLibrary.LoadComputeShader("../Atom/shaders/BRDFShader.hlsl");

        // Load pipelines

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

        // Generate BRDF texture
        ms_BRDFTexture = Renderer::CreateBRDFTexture();

        // Create common engine resources
        EngineResources::Initialize();

        // Wait for all compute operations to complete before we continue
        Device::Get().GetCommandQueue(CommandQueueType::Compute)->Flush();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::Shutdown()
    {
        ms_PipelineLibrary.Clear();
        ms_ShaderLibrary.Clear();
        ms_BRDFTexture.reset();

        EngineResources::Shutdown();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::BeginFrame()
    {
        u32 currentFrameIndex = GetCurrentFrameIndex();

        Device::Get().ProcessDeferredReleases(currentFrameIndex);
        PIXBeginEvent(Device::Get().GetCommandQueue(CommandQueueType::Graphics)->GetD3DCommandQueue().Get(), 0, "Begin Frame");
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::RenderMesh(Ref<CommandBuffer> commandBuffer, Ref<Mesh> mesh, u32 submeshIdx, Ref<Material> overrideMaterial)
    {
        const Submesh& submesh = mesh->GetSubmeshes()[submeshIdx];
        Ref<Material> material = overrideMaterial ? overrideMaterial : mesh->GetMaterialTable()->GetMaterial(submesh.MaterialIndex)->GetResource();

        // Transition textures
        for (auto& [_, texture] : material->GetTextures())
            if(texture)
                commandBuffer->TransitionResource(texture.get(), ResourceState::PixelShaderRead);

        // Set material descriptor tables
        if (material->IsDirty())
            material->UpdateDescriptorTables();

        commandBuffer->SetGraphicsConstants(ShaderBindPoint::Material, material->GetConstantsData().data(), material->GetConstantsData().size() / 4);
        commandBuffer->SetGraphicsDescriptorTables(ShaderBindPoint::Material, material->GetResourceTable(), material->GetSamplerTable());
        commandBuffer->SetVertexBuffer(mesh->GetVertexBuffer().get());
        commandBuffer->SetIndexBuffer(mesh->GetIndexBuffer().get());
        commandBuffer->DrawIndexed(submesh.IndexCount, 1, submesh.StartIndex, submesh.StartVertex, 0);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::RenderFullscreenQuad(Ref<CommandBuffer> commandBuffer, Texture* texture)
    {
        if (texture)
        {
            DescriptorAllocation resourceTable = Device::Get().GetGPUDescriptorHeap(DescriptorHeapType::ShaderResource)->AllocateTransient(1);
            Device::Get().CopyDescriptors(resourceTable, 1, &texture->GetSRV()->GetDescriptor(), DescriptorHeapType::ShaderResource);

            DescriptorAllocation samplerTable = Device::Get().GetGPUDescriptorHeap(DescriptorHeapType::Sampler)->AllocateTransient(1);
            Device::Get().CopyDescriptors(samplerTable, 1, &Renderer::GetSampler(TextureFilter::Linear, TextureWrap::Clamp)->GetDescriptor(), DescriptorHeapType::Sampler);

            commandBuffer->SetGraphicsDescriptorTables(ShaderBindPoint::Instance, resourceTable, samplerTable);
        }

        commandBuffer->SetVertexBuffer(EngineResources::QuadVertexBuffer.get());
        commandBuffer->SetIndexBuffer(EngineResources::QuadIndexBuffer.get());
        commandBuffer->DrawIndexed(EngineResources::QuadIndexBuffer->GetElementCount(), 1, 0, 0, 0);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<Texture> Renderer::CreateEnvironmentMap(Ref<Texture> equirectTexture, u32 mapSize, const char* debugName)
    {
        GPUDescriptorHeap* resourceHeap = Device::Get().GetGPUDescriptorHeap(DescriptorHeapType::ShaderResource);
        GPUDescriptorHeap* samplerHeap = Device::Get().GetGPUDescriptorHeap(DescriptorHeapType::Sampler);

        CommandQueue* computeQueue = Device::Get().GetCommandQueue(CommandQueueType::Compute);
        CommandQueue* copyQueue = Device::Get().GetCommandQueue(CommandQueueType::Copy);

        Ref<Fence> copyQueueFence = CreateRef<Fence>("CreateEnvironmentMap_CopyQueueFence");
        Ref<Fence> computeQueueFence = CreateRef<Fence>("CreateEnvironmentMap_ComputeQueueFence");

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
            computeCmdBuffer->TransitionResource(equirectTexture.get(), ResourceState::NonPixelShaderRead);
            computeCmdBuffer->TransitionResource(envMapUnfiltered.get(), ResourceState::UnorderedAccess);
            computeCmdBuffer->SetComputePipeline(ms_PipelineLibrary.Get<ComputePipeline>("EquirectToCubeMapPipeline").get());
            computeCmdBuffer->SetDescriptorHeaps(resourceHeap, samplerHeap);

            for (u32 mip = 0; mip < envMapDesc.MipLevels; mip++)
            {
                Ref<Texture> mipView = CreateRef<Texture>(*envMapUnfiltered, mip, UINT32_MAX);
                D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptors[] = { equirectTexture->GetSRV()->GetDescriptor(), mipView->GetUAV()->GetDescriptor() };

                DescriptorAllocation resourceTable = resourceHeap->AllocateTransient(_countof(cpuDescriptors));
                Device::Get().CopyDescriptors(resourceTable, _countof(cpuDescriptors), cpuDescriptors, DescriptorHeapType::ShaderResource);

                computeCmdBuffer->SetComputeConstants(ShaderBindPoint::Instance, &mip, 1);
                computeCmdBuffer->SetComputeDescriptorTables(ShaderBindPoint::Instance, resourceTable, samplerTable);
                computeCmdBuffer->Dispatch(envMapDesc.Width / 32, envMapDesc.Height / 32, 6);
            }

            computeCmdBuffer->TransitionResource(envMapUnfiltered.get(), ResourceState::Common);
            computeCmdBuffer->End();
            computeQueue->ExecuteCommandList(computeCmdBuffer);
            computeQueue->SignalFence(computeQueueFence, computeQueueFence->IncrementTargetValue());

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
            copyQueue->WaitFence(computeQueueFence, computeQueueFence->GetTargetValue());
            copyQueue->ExecuteCommandList(copyCmdBuffer);
            copyQueue->SignalFence(copyQueueFence, copyQueueFence->IncrementTargetValue());
        }

        {
            PIXBeginEvent(computeQueue->GetD3DCommandQueue().Get(), 0, "CubeMapPreFilter");

            DescriptorAllocation samplerTable = samplerHeap->AllocateTransient(1);
            Device::Get().CopyDescriptors(samplerTable, 1, &Renderer::GetSampler(TextureFilter::Linear, TextureWrap::Repeat)->GetDescriptor(), DescriptorHeapType::Sampler);

            Ref<CommandBuffer> computeCmdBuffer = computeQueue->GetCommandBuffer();
            computeCmdBuffer->Begin();
            computeCmdBuffer->TransitionResource(envMapUnfiltered.get(), ResourceState::NonPixelShaderRead);
            computeCmdBuffer->TransitionResource(envMap.get(), ResourceState::UnorderedAccess);
            computeCmdBuffer->SetComputePipeline(ms_PipelineLibrary.Get<ComputePipeline>("CubeMapPrefilterPipeline").get());
            computeCmdBuffer->SetDescriptorHeaps(resourceHeap, samplerHeap);

            u32 width = glm::max(envMapDesc.Width / 2, 1u);
            u32 height = glm::max(envMapDesc.Height / 2, 1u);

            for (u32 mip = 1; mip < envMapDesc.MipLevels; mip++)
            {
                Ref<Texture> mipView = CreateRef<Texture>(*envMap, mip, UINT32_MAX);
                D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptors[] = { envMapUnfiltered->GetSRV()->GetDescriptor(), mipView->GetUAV()->GetDescriptor() };

                DescriptorAllocation resourceTable = resourceHeap->AllocateTransient(_countof(cpuDescriptors));
                Device::Get().CopyDescriptors(resourceTable, _countof(cpuDescriptors), cpuDescriptors, DescriptorHeapType::ShaderResource);

                f32 roughness = mip / glm::max(envMapDesc.MipLevels - 1.0f, 1.0f);
                computeCmdBuffer->SetComputeConstants(ShaderBindPoint::Instance, &roughness, 1);
                computeCmdBuffer->SetComputeDescriptorTables(ShaderBindPoint::Instance, resourceTable, samplerTable);
                computeCmdBuffer->Dispatch(glm::max(width / 32, 1u), glm::max(height / 32, 1u), 6);

                width = glm::max(width / 2, 1u);
                height = glm::max(height / 2, 1u);
            }

            computeCmdBuffer->TransitionResource(envMap.get(), ResourceState::Common);
            computeCmdBuffer->End();
            computeQueue->WaitFence(copyQueueFence, copyQueueFence->GetTargetValue());
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
        gfxCmdBuffer->TransitionResource(environmentMap.get(), ResourceState::NonPixelShaderRead);
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
        computeCmdBuffer->TransitionResource(irradianceMap.get(), ResourceState::UnorderedAccess);
        computeCmdBuffer->SetComputePipeline(ms_PipelineLibrary.Get<ComputePipeline>("CubeMapIrradiancePipeline").get());
        computeCmdBuffer->SetDescriptorHeaps(resourceHeap, samplerHeap);
        computeCmdBuffer->SetComputeDescriptorTables(ShaderBindPoint::Instance, resourceTable, samplerTable);
        computeCmdBuffer->Dispatch(glm::max(irradianceMapDesc.Width / 32, 1u), glm::max(irradianceMapDesc.Height / 32, 1u), 6);
        computeCmdBuffer->TransitionResource(environmentMap.get(), ResourceState::Common);
        computeCmdBuffer->TransitionResource(irradianceMap.get(), ResourceState::Common);
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
        computeCmdBuffer->TransitionResource(brdfTexture.get(), ResourceState::UnorderedAccess);
        computeCmdBuffer->SetComputePipeline(ms_PipelineLibrary.Get<ComputePipeline>("BRDFPipeline").get());
        computeCmdBuffer->SetDescriptorHeaps(resourceHeap, nullptr);
        computeCmdBuffer->SetComputeDescriptorTables(ShaderBindPoint::Instance, resourceTable);
        computeCmdBuffer->Dispatch(glm::max(brdfDesc.Width / 32, 1u), glm::max(brdfDesc.Height / 32, 1u), 1);
        computeCmdBuffer->TransitionResource(brdfTexture.get(), ResourceState::Common);
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
            Ref<Texture> mipView = CreateRef<Texture>(*texture, mip, UINT32_MAX);
            D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptors[] = { texture->GetSRV()->GetDescriptor(), mipView->GetUAV()->GetDescriptor() };

            DescriptorAllocation resourceTable = resourceHeap->AllocateTransient(_countof(cpuDescriptors));
            Device::Get().CopyDescriptors(resourceTable, _countof(cpuDescriptors), cpuDescriptors, DescriptorHeapType::ShaderResource);

            GenerateMipCB constants;
            constants.TexelSize = { 1.0f / width, 1.0f / height };
            constants.TopMipLevel = mip - 1;

            computeCmdBuffer->TransitionResource(texture.get(), ResourceState::NonPixelShaderRead, mip - 1);
            computeCmdBuffer->TransitionResource(texture.get(), ResourceState::UnorderedAccess, mip);
            computeCmdBuffer->SetComputeConstants(ShaderBindPoint::Instance, &constants, 3);
            computeCmdBuffer->SetComputeDescriptorTables(ShaderBindPoint::Instance, resourceTable, samplerTable);
            computeCmdBuffer->Dispatch(glm::max(width / 8, 1u), glm::max(height / 8, 1u), 1);

            computeCmdBuffer->AddUAVBarrier(texture.get());

            width = glm::max(width / 2, 1u);
            height = glm::max(height / 2, 1u);
        }

        for(u32 mip = 0; mip < texture->GetMipLevels(); mip++)
            computeCmdBuffer->TransitionResource(texture.get(), ResourceState::Common, mip);

        computeCmdBuffer->End();

        computeQueue->ExecuteCommandList(computeCmdBuffer);
        computeQueue->Flush();

        PIXEndEvent(computeQueue->GetD3DCommandQueue().Get());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::UploadBufferData(Ref<Buffer> buffer, const void* srcData)
    {
        if (srcData)
        {
            u32 bufferSize = GetRequiredIntermediateSize(buffer->GetD3DResource().Get(), 0, 1);
            CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

            ComPtr<ID3D12Resource> uploadBuffer = nullptr;
            DXCall(Device::Get().GetD3DDevice()->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadBuffer)));

#if defined (ATOM_DEBUG)
            DXCall(uploadBuffer->SetName(L"Upload Buffer"));
#endif
            D3D12_SUBRESOURCE_DATA subresourceData = {};
            subresourceData.pData = srcData;
            subresourceData.RowPitch = buffer->GetSize();
            subresourceData.SlicePitch = subresourceData.RowPitch;

            CommandQueue* copyQueue = Device::Get().GetCommandQueue(CommandQueueType::Copy);
            Ref<CommandBuffer> copyCommandBuffer = copyQueue->GetCommandBuffer();
            copyCommandBuffer->Begin();
            copyCommandBuffer->TransitionResource(buffer.get(), ResourceState::CopyDestination);
            copyCommandBuffer->CommitBarriers();
            UpdateSubresources<1>(copyCommandBuffer->GetCommandList().Get(), buffer->GetD3DResource().Get(), uploadBuffer.Get(), 0, 0, 1, &subresourceData);
            copyCommandBuffer->End();
            copyQueue->ExecuteCommandList(copyCommandBuffer);
            copyQueue->Flush();
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::UploadTextureData(Ref<Texture> texture, const void* srcData, u32 mip, u32 slice)
    {
        if (srcData)
        {
            u32 subresourceIdx = Texture::CalculateSubresource(mip, slice, texture->GetMipLevels(), texture->GetArraySize());
            u32 bufferSize = GetRequiredIntermediateSize(texture->GetD3DResource().Get(), subresourceIdx, 1);
            CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

            CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
            ComPtr<ID3D12Resource> uploadBuffer = nullptr;
            DXCall(Device::Get().GetD3DDevice()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadBuffer)));

#if defined (ATOM_DEBUG)
            DXCall(uploadBuffer->SetName(L"Upload Buffer"));
#endif

            u32 width = glm::max(texture->GetWidth() >> mip, 1u);
            u32 height = glm::max(texture->GetHeight() >> mip, 1u);

            D3D12_SUBRESOURCE_DATA subresourceData = {};
            subresourceData.pData = srcData;
            subresourceData.RowPitch = ((width * Utils::GetTextureFormatSize(texture->GetFormat()) + 255) / 256) * 256;
            subresourceData.SlicePitch = height * subresourceData.RowPitch;

            CommandQueue* copyQueue = Device::Get().GetCommandQueue(CommandQueueType::Copy);
            Ref<CommandBuffer> copyCommandBuffer = copyQueue->GetCommandBuffer();
            copyCommandBuffer->Begin();
            copyCommandBuffer->TransitionResource(texture.get(), ResourceState::CopyDestination);
            copyCommandBuffer->CommitBarriers();
            UpdateSubresources<1>(copyCommandBuffer->GetCommandList().Get(), texture->GetD3DResource().Get(), uploadBuffer.Get(), 0, subresourceIdx, 1, &subresourceData);
            copyCommandBuffer->End();
            copyQueue->ExecuteCommandList(copyCommandBuffer);
            copyQueue->Flush();
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<ReadbackBuffer> Renderer::ReadbackTextureData(Ref<Texture> texture, u32 mip, u32 slice)
    {
        u32 subresourceIdx = Texture::CalculateSubresource(mip, slice, texture->GetMipLevels(), texture->GetArraySize());

        BufferDescription readbackBufferDesc;
        readbackBufferDesc.ElementSize = GetRequiredIntermediateSize(texture->GetD3DResource().Get(), subresourceIdx, 1);
        readbackBufferDesc.ElementCount = 1;
        readbackBufferDesc.IsDynamic = true;

        Ref<ReadbackBuffer> readbackBuffer = CreateRef<ReadbackBuffer>(readbackBufferDesc, "Readback Buffer");

        CommandQueue* copyQueue = Device::Get().GetCommandQueue(CommandQueueType::Copy);
        Ref<CommandBuffer> copyCommandBuffer = copyQueue->GetCommandBuffer();
        copyCommandBuffer->Begin();
        copyCommandBuffer->TransitionResource(texture.get(), ResourceState::CopySource);
        copyCommandBuffer->CommitBarriers();
        copyCommandBuffer->CopyTexture(texture.get(), readbackBuffer.get(), subresourceIdx);
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
    ShaderLibrary& Renderer::GetShaderLibrary()
    {
        return ms_ShaderLibrary;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    PipelineLibrary& Renderer::GetPipelineLibrary()
    {
        return ms_PipelineLibrary;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<Texture> Renderer::GetBRDF()
    {
        return ms_BRDFTexture;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<TextureSampler> Renderer::GetSampler(TextureFilter filter, TextureWrap wrap)
    {
        if (filter == TextureFilter::Linear)
        {
            if (wrap == TextureWrap::Clamp)
                return EngineResources::LinearClampSampler;
            else if (wrap == TextureWrap::Repeat)
                return EngineResources::LinearRepeatSampler;
        }
        else if (filter == TextureFilter::Nearest)
        {
            if (wrap == TextureWrap::Clamp)
                return EngineResources::NearestClampSampler;
            else if (wrap == TextureWrap::Repeat)
                return EngineResources::NearestRepeatSampler;
        }
        else if (filter == TextureFilter::Anisotropic)
        {
            if (wrap == TextureWrap::Clamp)
                return EngineResources::AnisotropicClampSampler;
            else if (wrap == TextureWrap::Repeat)
                return EngineResources::AnisotropicRepeatSampler;
        }

        ATOM_ENGINE_ASSERT(false);
        return EngineResources::LinearClampSampler;
    }
}