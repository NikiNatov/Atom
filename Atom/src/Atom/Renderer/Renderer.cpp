#include "atompch.h"
#include "Renderer.h"

#include "Atom/Core/Application.h"
#include "Atom/Core/DirectX12/DirectX12Utils.h"
#include "Atom/Asset/MeshAsset.h"

#include "Atom/Renderer/CommandQueue.h"
#include "Atom/Renderer/EngineResources.h"

#include "Atom/Renderer/RenderPasses/SkyBoxPass.h"
#include "Atom/Renderer/RenderPasses/GeometryPass.h"
#include "Atom/Renderer/RenderPasses/CompositePass.h"

#include <autogen/cpp/FrameParams.h>
#include <autogen/cpp/EquirectToCubeMapParams.h>
#include <autogen/cpp/CubeMapPrefilterParams.h>
#include <autogen/cpp/CubeMapIrradianceParams.h>
#include <autogen/cpp/GenerateMipsParams.h>

#include <imgui.h>
#include <pix3.h>

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    Renderer::Renderer(const RendererSpecification& spec)
        : m_Specification(spec)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::BeginScene(const Camera& camera, const glm::mat4& cameraTransform, const Ref<Texture>& environmentMap, const Ref<Texture>& irradianceMap)
    {
        u32 currentFrameIdx = Application::Get().GetCurrentFrameIndex();

        m_FrameData.ViewMatrix = glm::inverse(cameraTransform);
        m_FrameData.ProjectionMatrix = camera.GetProjection();
        m_FrameData.InvViewProjMatrix = glm::inverse(m_FrameData.ProjectionMatrix * m_FrameData.ViewMatrix);
        m_FrameData.CameraPosition = cameraTransform[3];
        m_FrameData.CameraExposure = 0.5f; // Hard-coded for now
        m_FrameData.Lights.clear();
        m_FrameData.BoneTransforms.clear();
        m_FrameData.StaticMeshes.clear();
        m_FrameData.AnimatedMeshes.clear();
        m_FrameData.EnvironmentMaps[currentFrameIdx] = environmentMap ? environmentMap : EngineResources::BlackTextureCube;
        m_FrameData.IrradianceMaps[currentFrameIdx] = irradianceMap ? irradianceMap : EngineResources::BlackTextureCube;

        m_RenderGraph.Reset();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::BeginScene(const EditorCamera& editorCamera, const Ref<Texture>& environmentMap, const Ref<Texture>& irradianceMap)
    {
        u32 currentFrameIdx = Application::Get().GetCurrentFrameIndex();

        m_FrameData.ViewMatrix = editorCamera.GetViewMatrix();
        m_FrameData.ProjectionMatrix = editorCamera.GetProjection();
        m_FrameData.InvViewProjMatrix = glm::inverse(m_FrameData.ProjectionMatrix * m_FrameData.ViewMatrix);
        m_FrameData.CameraPosition = editorCamera.GetPosition();
        m_FrameData.CameraExposure = 0.5f; // Hard-coded for now
        m_FrameData.Lights.clear();
        m_FrameData.BoneTransforms.clear();
        m_FrameData.StaticMeshes.clear();
        m_FrameData.AnimatedMeshes.clear();
        m_FrameData.EnvironmentMaps[currentFrameIdx] = environmentMap ? environmentMap : EngineResources::BlackTextureCube;
        m_FrameData.IrradianceMaps[currentFrameIdx] = irradianceMap ? irradianceMap : EngineResources::BlackTextureCube;

        m_RenderGraph.Reset();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::SubmitDirectionalLight(const glm::vec3& color, const glm::vec3& direction, f32 intensity)
    {
        Light& light = m_FrameData.Lights.emplace_back();
        light.Type = LightType::DirLight;
        light.Color = { color.r, color.g, color.b, 1.0 };
        light.Direction = { direction.x, direction.y, direction.z, 0.0f };
        light.Intensity = intensity;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::SubmitPointLight(const glm::vec3& color, const glm::vec3& position, f32 intensity, const glm::vec3& attenuationFactors)
    {
        Light& light = m_FrameData.Lights.emplace_back();
        light.Type = LightType::PointLight;
        light.Color = { color.r, color.g, color.b, 1.0 };
        light.Position = { position.x, position.y, position.z, 1.0f };
        light.Intensity = intensity;
        light.AttenuationFactors = attenuationFactors;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::SubmitSpotLight(const glm::vec3& color, const glm::vec3& position, const glm::vec3& direction, f32 intensity, f32 coneAngle, const glm::vec3& attenuationFactors)
    {
        Light& light = m_FrameData.Lights.emplace_back();
        light.Type = LightType::SpotLight;
        light.Color = { color.r, color.g, color.b, 1.0 };
        light.Position = { position.x, position.y, position.z, 1.0f };
        light.Direction = { direction.x, direction.y, direction.z, 0.0f };
        light.Intensity = intensity;
        light.ConeAngle = coneAngle;
        light.AttenuationFactors = attenuationFactors;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::SubmitMesh(const Ref<Mesh>& mesh, const glm::mat4& transform, const Ref<MaterialTable>& materialTable)
    {
        if (!mesh)
            return;

        u32 currentFrameIdx = Application::Get().GetCurrentFrameIndex();

        const auto& submeshes = mesh->GetSubmeshes();
        for (u32 submeshIdx = 0; submeshIdx < submeshes.size(); submeshIdx++)
        {
            const Submesh& submesh = submeshes[submeshIdx];
            const Ref<MaterialTable>& meshMaterialTable = mesh->GetMaterialTable();
            Ref<Material> material = materialTable && materialTable->HasMaterial(submesh.MaterialIndex) ? materialTable->GetMaterial(submesh.MaterialIndex) : meshMaterialTable->GetMaterial(submesh.MaterialIndex);

            MeshEntry& meshEntry = m_FrameData.StaticMeshes.emplace_back();
            meshEntry.Mesh = mesh;
            meshEntry.SubmeshIndex = submeshIdx;
            meshEntry.Transform = transform;
            meshEntry.Material = material ? material : EngineResources::ErrorMaterial;
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::SubmitAnimatedMesh(const Ref<Mesh>& mesh, const glm::mat4& transform, const Ref<MaterialTable>& materialTable, const Ref<Skeleton>& skeleton)
    {
        if (!mesh || !skeleton)
            return;

        u32 currentFrameIdx = Application::Get().GetCurrentFrameIndex();

        // Submit draw command for each submesh
        const auto& submeshes = mesh->GetSubmeshes();
        for (u32 submeshIdx = 0; submeshIdx < submeshes.size(); submeshIdx++)
        {
            const Submesh& submesh = submeshes[submeshIdx];
            const Ref<MaterialTable>& meshMaterialTable = mesh->GetMaterialTable();
            Ref<Material> material = materialTable && materialTable->HasMaterial(submesh.MaterialIndex) ? materialTable->GetMaterial(submesh.MaterialIndex) : meshMaterialTable->GetMaterial(submesh.MaterialIndex);

            MeshEntry& meshEntry = m_FrameData.AnimatedMeshes.emplace_back();
            meshEntry.Mesh = mesh;
            meshEntry.SubmeshIndex = submeshIdx;
            meshEntry.Transform = transform;
            meshEntry.Material = material ? material : EngineResources::ErrorMaterialAnimated;
            meshEntry.BoneTransformOffset = m_FrameData.BoneTransforms.size();
        }

        // Set all bone transforms
        for (auto& bone : skeleton->GetBones())
            m_FrameData.BoneTransforms.push_back(bone.AnimatedTransform);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::SetViewportSize(u32 width, u32 height)
    {
        m_FrameData.ViewportWidth = width;
        m_FrameData.ViewportHeight = height;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::Render()
    {
        BuildRenderPasses();
        UpdateFrameGPUBuffers();
        RecordCommandBuffers();
        ExecuteCommandBuffers();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::OnImGuiRender()
    {
        ImGui::Begin("Scene Renderer");

        for (RenderPassID passID : m_RenderGraph.GetOrderedPasses())
        {
            if (ImGui::CollapsingHeader(m_RenderGraph.GetRenderPass(passID)->GetName().c_str()))
            {
                const ResourceScheduler& resourceScheduler = m_ResourceSchedulers[Application::Get().GetCurrentFrameIndex()];
                for (const IResourceView* outputView : resourceScheduler.GetPassOutputs(passID))
                {
                    Resource* resource = resourceScheduler.GetResource(outputView->GetResourceID());

                    ImGui::Columns(2);
                    ImGui::SetColumnWidth(0, 150.0f);
                    ImGui::Text(resource->GetName());
                    ImGui::NextColumn();
                    ImGui::PushItemWidth(-1);

                    if (TextureResource* textureResource = resource->As<TextureResource>())
                    {
                        ImGui::ImageButton((ImTextureID)textureResource->GetHWResource(), { textureResource->GetWidth() * 0.25f, textureResource->GetHeight() * 0.25f }, { 0.0f, 0.0f });
                    }
                    else if (RenderSurfaceResource* surfaceResource = resource->As<RenderSurfaceResource>())
                    {
                        ImGui::ImageButton((ImTextureID)surfaceResource->GetHWResource(), { surfaceResource->GetWidth() * 0.25f, surfaceResource->GetHeight() * 0.25f }, { 0.0f, 0.0f });
                    }

                    ImGui::PopItemWidth();
                    ImGui::Columns(1);

                    ImGui::Separator();
                }
            }
        }

        ImGui::End();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const Texture* Renderer::GetFinalImage() const
    {
        const ResourceScheduler& resourceScheduler = m_ResourceSchedulers[Application::Get().GetCurrentFrameIndex()];

        ATOM_ENGINE_ASSERT(!m_RenderGraph.GetOrderedPasses().empty(), "Render graph has no render passes");
        RenderPassID finalPassID = m_RenderGraph.GetOrderedPasses().back();
        const ResourceID& finalOutputResourceID = resourceScheduler.GetPassOutputs(finalPassID).back()->GetResourceID();
        const RenderSurfaceResource* finalOutput = resourceScheduler.GetResource(finalOutputResourceID)->As<RenderSurfaceResource>();

        ATOM_ENGINE_ASSERT(finalOutput);
        return (Texture*)finalOutput->GetHWResource();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::BuildRenderPasses()
    {
        m_RenderGraph.AddRenderPass<SkyBoxPass>("SkyBoxPass", m_FrameData.ViewportWidth, m_FrameData.ViewportHeight);
        m_RenderGraph.AddRenderPass<GeometryPass>("StaticGeometryPass", m_FrameData.StaticMeshes, false);
        m_RenderGraph.AddRenderPass<GeometryPass>("AnimatedGeometryPass", m_FrameData.AnimatedMeshes, true);
        m_RenderGraph.AddRenderPass<CompositePass>("CompositePass", m_FrameData.ViewportWidth, m_FrameData.ViewportHeight, m_Specification.RenderToSwapChain);

        m_RenderGraph.Build(m_ResourceSchedulers[Application::Get().GetCurrentFrameIndex()]);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::UpdateFrameGPUBuffers()
    {
        u32 currentFrameIdx = Application::Get().GetCurrentFrameIndex();

        // Update lights structured buffer data
        if (!m_FrameData.Lights.empty())
        {
            if (!m_FrameData.LightsGPUBuffers[currentFrameIdx] || m_FrameData.LightsGPUBuffers[currentFrameIdx]->GetElementCount() != m_FrameData.Lights.size())
            {
                BufferDescription sbDesc;
                sbDesc.ElementCount = m_FrameData.Lights.size();
                sbDesc.ElementSize = sizeof(Light);
                sbDesc.IsDynamic = true;

                m_FrameData.LightsGPUBuffers[currentFrameIdx] = CreateRef<StructuredBuffer>(sbDesc, "LightsGPUBuffer");
            }

            void* lightsData = m_FrameData.LightsGPUBuffers[currentFrameIdx]->Map(0, 0);
            memcpy(lightsData, m_FrameData.Lights.data(), sizeof(Light) * m_FrameData.Lights.size());
            m_FrameData.LightsGPUBuffers[currentFrameIdx]->Unmap();
        }

        // Update bone transforms structured buffer data
        if (!m_FrameData.BoneTransforms.empty())
        {
            if (!m_FrameData.BoneTransformsGPUBuffers[currentFrameIdx] || m_FrameData.BoneTransformsGPUBuffers[currentFrameIdx]->GetElementCount() != m_FrameData.BoneTransforms.size())
            {
                BufferDescription animSBDesc;
                animSBDesc.ElementCount = m_FrameData.BoneTransforms.size();
                animSBDesc.ElementSize = sizeof(glm::mat4);
                animSBDesc.IsDynamic = true;

                m_FrameData.BoneTransformsGPUBuffers[currentFrameIdx] = CreateRef<StructuredBuffer>(animSBDesc, "BoneTransformsGPUBuffer");
            }

            void* boneTransformData = m_FrameData.BoneTransformsGPUBuffers[currentFrameIdx]->Map(0, 0);
            memcpy(boneTransformData, m_FrameData.BoneTransforms.data(), sizeof(glm::mat4) * m_FrameData.BoneTransforms.size());
            m_FrameData.BoneTransformsGPUBuffers[currentFrameIdx]->Unmap();
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::RecordCommandBuffers()
    {
        u32 currentFrameIdx = Application::Get().GetCurrentFrameIndex();

        SIG::FrameParams frameSIG;
        frameSIG.SetViewMatrix(m_FrameData.ViewMatrix);
        frameSIG.SetProjectionMatrix(m_FrameData.ProjectionMatrix);
        frameSIG.SetInvViewProjMatrix(m_FrameData.InvViewProjMatrix);
        frameSIG.SetCameraPosition(m_FrameData.CameraPosition);
        frameSIG.SetCameraExposure(m_FrameData.CameraExposure);
        frameSIG.SetNumLights(m_FrameData.Lights.size());
        frameSIG.SetLights(m_FrameData.LightsGPUBuffers[currentFrameIdx].get());
        frameSIG.SetBoneTransforms(m_FrameData.BoneTransformsGPUBuffers[currentFrameIdx].get());
        frameSIG.SetEnvironmentMap(m_FrameData.EnvironmentMaps[currentFrameIdx].get());
        frameSIG.SetIrradianceMap(m_FrameData.IrradianceMaps[currentFrameIdx].get());
        frameSIG.SetBRDFMap(EngineResources::BRDFTexture.get());
        frameSIG.SetEnvironmentMapSampler(EngineResources::LinearClampSampler.get());
        frameSIG.SetIrradianceMapSampler(EngineResources::LinearClampSampler.get());
        frameSIG.SetBRDFMapSampler(EngineResources::LinearClampSampler.get());

        frameSIG.Compile();

        GPUDescriptorHeap* resourceHeap = Device::Get().GetGPUDescriptorHeap(DescriptorHeapType::ShaderResource);
        GPUDescriptorHeap* samplerHeap = Device::Get().GetGPUDescriptorHeap(DescriptorHeapType::Sampler);

        for (u32 queueIdx = 0; queueIdx < (u32)CommandQueueType::NumTypes; queueIdx++)
        {
            for (const RenderGraph::RenderGraphEvent& event : m_RenderGraph.GetRenderGraphEvents((CommandQueueType)queueIdx))
            {
                if (const RenderGraph::RedirectedTransitionsEvent* transitionEventPtr = std::get_if<RenderGraph::RedirectedTransitionsEvent>(&event))
                {
                    transitionEventPtr->CmdBuffer->Begin();
                    PIXBeginEvent(transitionEventPtr->CmdBuffer->GetCommandList().Get(), 0, fmt::format("RedirectedTransitions_DepGroup{}", transitionEventPtr->DepGroupIndex).c_str());

                    for (const TransitionBarrier& barrier : *(transitionEventPtr->RedirectedTransitionBarriers))
                        transitionEventPtr->CmdBuffer->TransitionResource(barrier.GetResource(), barrier.GetAfterState());

                    PIXEndEvent(transitionEventPtr->CmdBuffer->GetCommandList().Get());
                    transitionEventPtr->CmdBuffer->End();
                }
                else if (const RenderGraph::RenderPassEvent* passEventPtr = std::get_if<RenderGraph::RenderPassEvent>(&event))
                {
                    passEventPtr->CmdBuffer->Begin();
                    PIXBeginEvent(passEventPtr->CmdBuffer->GetCommandList().Get(), 0, passEventPtr->RenderPass->GetName().c_str());

                    for (const TransitionBarrier& barrier : *(passEventPtr->TransitionBarriers))
                        passEventPtr->CmdBuffer->TransitionResource(barrier.GetResource(), barrier.GetAfterState());

                    for (const UAVBarrier& barrier : *(passEventPtr->UAVBarriers))
                        passEventPtr->CmdBuffer->AddUAVBarrier(barrier.GetResource());

                    passEventPtr->CmdBuffer->SetDescriptorHeaps(resourceHeap, samplerHeap);

                    // Set pipeline state and Frame SIG
                    const ResourceScheduler& resourceScheduler = m_ResourceSchedulers[currentFrameIdx];
                    const Pipeline* pipeline = resourceScheduler.GetPassPipeline(passEventPtr->RenderPass->GetID());
                    ATOM_ENGINE_ASSERT(pipeline, fmt::format("No pipline set for pass {}", passEventPtr->RenderPass->GetName().c_str()).c_str());

                    if (const GraphicsPipeline* gfxPipeline = dynamic_cast<const GraphicsPipeline*>(pipeline))
                    {
                        passEventPtr->CmdBuffer->SetGraphicsPipeline(gfxPipeline);
                        passEventPtr->CmdBuffer->SetGraphicsConstants(ShaderBindPoint::Frame, frameSIG.GetConstantBuffer().get());
                        passEventPtr->CmdBuffer->SetGraphicsDescriptorTables(ShaderBindPoint::Frame, frameSIG.GetResourceTable(), frameSIG.GetSamplerTable());
                    }
                    else if (const ComputePipeline* computePipeline = dynamic_cast<const ComputePipeline*>(pipeline))
                    {
                        passEventPtr->CmdBuffer->SetComputePipeline(computePipeline);
                        passEventPtr->CmdBuffer->SetComputeConstants(ShaderBindPoint::Frame, frameSIG.GetConstantBuffer().get());
                        passEventPtr->CmdBuffer->SetComputeDescriptorTables(ShaderBindPoint::Frame, frameSIG.GetResourceTable(), frameSIG.GetSamplerTable());
                    }

                    RenderPassContext passContext(passEventPtr->RenderPass->GetID(), passEventPtr->CmdBuffer, resourceScheduler);
                    passEventPtr->RenderPass->Execute(passContext);

                    PIXEndEvent(passEventPtr->CmdBuffer->GetCommandList().Get());
                    passEventPtr->CmdBuffer->End();
                }
            }
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Renderer::ExecuteCommandBuffers()
    {
        Vector<Ref<CommandBuffer>> cmdBufferBatches[u32(CommandQueueType::NumTypes)];

        for (u32 queueIdx = 0; queueIdx < (u32)CommandQueueType::NumTypes; queueIdx++)
        {
            CommandQueue* cmdQueue = Device::Get().GetCommandQueue(CommandQueueType(queueIdx));

            for (const RenderGraph::RenderGraphEvent& event : m_RenderGraph.GetRenderGraphEvents((CommandQueueType)queueIdx))
            {
                if (const RenderGraph::RedirectedTransitionsEvent* transitionEventPtr = std::get_if<RenderGraph::RedirectedTransitionsEvent>(&event))
                {
                    if (!transitionEventPtr->SignalsToWait.empty())
                    {
                        // If we have fences to wait for, execute the current cmd buffer batch and wait
                        cmdQueue->ExecuteCommandLists(cmdBufferBatches[queueIdx]);
                        cmdBufferBatches[queueIdx].clear();

                        for (u32 i = 0; i < transitionEventPtr->SignalsToWait.size(); i++)
                            cmdQueue->WaitFence(transitionEventPtr->SignalsToWait[i]->Fence, transitionEventPtr->SignalsToWait[i]->FenceValue);
                    }

                    // Execute redirected transitions and signal fence
                    cmdQueue->ExecuteCommandList(transitionEventPtr->CmdBuffer);
                    cmdQueue->SignalFence(transitionEventPtr->Signal.Fence, transitionEventPtr->Signal.FenceValue);
                }
                else if (const RenderGraph::RenderPassEvent* passEventPtr = std::get_if<RenderGraph::RenderPassEvent>(&event))
                {
                    if (!passEventPtr->SignalsToWait.empty())
                    {
                        // If we have fences to wait for, flush the current cmd buffer batch and wait
                        cmdQueue->ExecuteCommandLists(cmdBufferBatches[queueIdx]);
                        cmdBufferBatches[queueIdx].clear();

                        for (u32 i = 0; i < passEventPtr->SignalsToWait.size(); i++)
                            cmdQueue->WaitFence(passEventPtr->SignalsToWait[i]->Fence, passEventPtr->SignalsToWait[i]->FenceValue);
                    }

                    if (passEventPtr->CmdBuffer)
                        cmdBufferBatches[queueIdx].push_back(passEventPtr->CmdBuffer);

                    if (passEventPtr->Signal.Fence)
                    {
                        // If we have fences to signal, flush the current cmd buffer batch and signal
                        cmdQueue->ExecuteCommandLists(cmdBufferBatches[queueIdx]);
                        cmdQueue->SignalFence(passEventPtr->Signal.Fence, passEventPtr->Signal.FenceValue);

                        cmdBufferBatches[queueIdx].clear();
                    }
                }
            }
        }

        // Flush any remaining cmd buffers
        for (u32 queueIdx = 0; queueIdx < (u32)CommandQueueType::NumTypes; queueIdx++)
        {
            CommandQueue* cmdQueue = Device::Get().GetCommandQueue(CommandQueueType(queueIdx));
            cmdQueue->ExecuteCommandLists(cmdBufferBatches[queueIdx]);
        }
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
        envMapDesc.InitialState = ResourceState::UnorderedAccess;

        Ref<Texture> envMapUnfiltered = CreateRef<Texture>(envMapDesc, fmt::format("{}(Unfiltered)", debugName).c_str());

        {
            ComputePipelineDescription pipelineDesc;
            pipelineDesc.Shader = ShaderLibrary::Get().Get<ComputeShader>("EquirectToCubeMap");
            Ref<ComputePipeline> pipeline = PipelineLibrary::Get().LoadComputePipeline(pipelineDesc, "EquirectToCubeMapPipeline");

            PIXBeginEvent(computeQueue->GetD3DCommandQueue().Get(), 0, "EquirectToCubeMap");

            SIG::EquirectToCubeMapParams params;
            params.SetInputTexture(equirectTexture.get());
            params.SetInputTextureSampler(EngineResources::LinearRepeatSampler.get());

            Ref<CommandBuffer> computeCmdBuffer = computeQueue->GetCommandBuffer();
            computeCmdBuffer->Begin();
            computeCmdBuffer->TransitionResource(equirectTexture.get(), ResourceState::NonPixelShaderRead);
            computeCmdBuffer->SetComputePipeline(pipeline.get());
            computeCmdBuffer->SetDescriptorHeaps(resourceHeap, samplerHeap);

            for (u32 mip = 0; mip < envMapDesc.MipLevels; mip++)
            {
                Ref<Texture> mipView = CreateRef<Texture>(*envMapUnfiltered, mip, UINT32_MAX);

                params.SetMipLevel(mip);
                params.SetOutputTexture(mipView.get());
                params.Compile();

                computeCmdBuffer->SetComputeConstants(ShaderBindPoint::Instance, params.GetRootConstantsData(), 1);
                computeCmdBuffer->SetComputeDescriptorTables(ShaderBindPoint::Instance, params.GetResourceTable(), params.GetSamplerTable());
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
            ComputePipelineDescription pipelineDesc;
            pipelineDesc.Shader = ShaderLibrary::Get().Get<ComputeShader>("CubeMapPrefilter");
            Ref<ComputePipeline> pipeline = PipelineLibrary::Get().LoadComputePipeline(pipelineDesc, "CubeMapPrefilterPipeline");

            PIXBeginEvent(computeQueue->GetD3DCommandQueue().Get(), 0, "CubeMapPreFilter");

            SIG::CubeMapPrefilterParams params;
            params.SetEnvMapUnfiltered(envMapUnfiltered.get());
            params.SetEnvMapSampler(EngineResources::LinearRepeatSampler.get());

            Ref<CommandBuffer> computeCmdBuffer = computeQueue->GetCommandBuffer();
            computeCmdBuffer->Begin();
            computeCmdBuffer->TransitionResource(envMapUnfiltered.get(), ResourceState::NonPixelShaderRead);
            computeCmdBuffer->SetComputePipeline(pipeline.get());
            computeCmdBuffer->SetDescriptorHeaps(resourceHeap, samplerHeap);

            u32 width = glm::max(envMapDesc.Width / 2, 1u);
            u32 height = glm::max(envMapDesc.Height / 2, 1u);

            for (u32 mip = 1; mip < envMapDesc.MipLevels; mip++)
            {
                Ref<Texture> mipView = CreateRef<Texture>(*envMap, mip, UINT32_MAX);
                f32 roughness = mip / glm::max(envMapDesc.MipLevels - 1.0f, 1.0f);

                params.SetEnvMap(mipView.get());
                params.SetRoughness(roughness);
                params.Compile();

                computeCmdBuffer->SetComputeConstants(ShaderBindPoint::Instance, params.GetRootConstantsData(), 1);
                computeCmdBuffer->SetComputeDescriptorTables(ShaderBindPoint::Instance, params.GetResourceTable(), params.GetSamplerTable());
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

        ComputePipelineDescription pipelineDesc;
        pipelineDesc.Shader = ShaderLibrary::Get().Get<ComputeShader>("CubeMapIrradiance");
        Ref<ComputePipeline> pipeline = PipelineLibrary::Get().LoadComputePipeline(pipelineDesc, "CubeMapIrradiancePipeline");

        TextureDescription irradianceMapDesc;
        irradianceMapDesc.Format = TextureFormat::RGBA16F;
        irradianceMapDesc.Width = mapSize;
        irradianceMapDesc.Height = mapSize;
        irradianceMapDesc.ArraySize = 6;
        irradianceMapDesc.MipLevels = 1;
        irradianceMapDesc.Flags = TextureFlags::UnorderedAccess | TextureFlags::ShaderResource | TextureFlags::CubeMap;

        Ref<Texture> irradianceMap = CreateRef<Texture>(irradianceMapDesc, fmt::format("{}(IrradianceMap)", debugName).c_str());

        PIXBeginEvent(computeQueue->GetD3DCommandQueue().Get(), 0, "GenerateIrradianceMap");

        SIG::CubeMapIrradianceParams params;
        params.SetEnvMap(environmentMap.get());
        params.SetIrradianceMap(irradianceMap.get());
        params.SetEnvMapSampler(EngineResources::LinearRepeatSampler.get());
        params.Compile();

        Ref<CommandBuffer> computeCmdBuffer = computeQueue->GetCommandBuffer();
        computeCmdBuffer->Begin();
        computeCmdBuffer->TransitionResource(irradianceMap.get(), ResourceState::UnorderedAccess);
        computeCmdBuffer->SetComputePipeline(pipeline.get());
        computeCmdBuffer->SetDescriptorHeaps(resourceHeap, samplerHeap);
        computeCmdBuffer->SetComputeDescriptorTables(ShaderBindPoint::Instance, params.GetResourceTable(), params.GetSamplerTable());
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
    void Renderer::GenerateMips(Ref<Texture> texture)
    {
        GPUDescriptorHeap* resourceHeap = Device::Get().GetGPUDescriptorHeap(DescriptorHeapType::ShaderResource);
        GPUDescriptorHeap* samplerHeap = Device::Get().GetGPUDescriptorHeap(DescriptorHeapType::Sampler);

        CommandQueue* computeQueue = Device::Get().GetCommandQueue(CommandQueueType::Compute);

        ComputePipelineDescription pipelineDesc;
        pipelineDesc.Shader = ShaderLibrary::Get().Get<ComputeShader>("GenerateMips");
        Ref<ComputePipeline> pipeline = PipelineLibrary::Get().LoadComputePipeline(pipelineDesc, "GenerateMipsPipeline");

        PIXBeginEvent(computeQueue->GetD3DCommandQueue().Get(), 0, "GenerateMips");

        SIG::GenerateMipsParams params;
        params.SetSrcTexture(texture.get());
        params.SetBilinearClamp(EngineResources::LinearClampSampler.get());

        // Run compute shader for each mip
        Ref<CommandBuffer> computeCmdBuffer = computeQueue->GetCommandBuffer();
        computeCmdBuffer->Begin();
        computeCmdBuffer->SetComputePipeline(pipeline.get());
        computeCmdBuffer->SetDescriptorHeaps(resourceHeap, samplerHeap);

        u32 width = glm::max(texture->GetWidth() / 2, 1u);
        u32 height = glm::max(texture->GetHeight() / 2, 1u);

        for (u32 mip = 1; mip < texture->GetMipLevels(); mip++)
        {
            Ref<Texture> mipView = CreateRef<Texture>(*texture, mip, UINT32_MAX);

            params.SetDstTexture(mipView.get());
            params.SetTexelSize({ 1.0f / width, 1.0f / height });
            params.SetTopMipLevel(mip - 1);
            params.Compile();

            computeCmdBuffer->TransitionResource(texture.get(), ResourceState::NonPixelShaderRead, mip - 1);
            computeCmdBuffer->TransitionResource(texture.get(), ResourceState::UnorderedAccess, mip);
            computeCmdBuffer->SetComputeConstants(ShaderBindPoint::Instance, params.GetRootConstantsData(), 3);
            computeCmdBuffer->SetComputeDescriptorTables(ShaderBindPoint::Instance, params.GetResourceTable(), params.GetSamplerTable());
            computeCmdBuffer->Dispatch(glm::max(width / 8, 1u), glm::max(height / 8, 1u), 1);

            computeCmdBuffer->AddUAVBarrier(texture.get());

            width = glm::max(width / 2, 1u);
            height = glm::max(height / 2, 1u);
        }

        for(u32 mip = 0; mip < texture->GetMipLevels(); mip++)
            computeCmdBuffer->TransitionResource(texture.get(), ResourceState::Common, mip);

        computeCmdBuffer->End();
        computeQueue->ExecuteCommandList(computeCmdBuffer);

        PIXEndEvent(computeQueue->GetD3DCommandQueue().Get());

        computeQueue->Flush();
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