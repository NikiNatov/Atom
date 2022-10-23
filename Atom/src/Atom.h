#pragma once

// Core
#include "Atom/Core/Application.h"
#include "Atom/Core/Core.h"
#include "Atom/Core/Logger.h"
#include "Atom/Core/Events/Events.h"
#include "Atom/Core/Layer.h"
#include "Atom/Core/LayerStack.h"
#include "Atom/Core/Timestep.h"
#include "Atom/Core/Timer.h"
#include "Atom/Core/Window.h"
#include "Atom/Core/Input.h"
#include "Atom/Core/DataStructures/ThreadSafeQueue.h"

// ImGui
#include "Atom/ImGui/ImGuiLayer.h"

// Renderer
#include "Atom/Renderer/Device.h"
#include "Atom/Renderer/CommandQueue.h"
#include "Atom/Renderer/SwapChain.h"
#include "Atom/Renderer/CommandBuffer.h"
#include "Atom/Renderer/Texture.h"
#include "Atom/Renderer/Shader.h"
#include "Atom/Renderer/Pipeline.h"
#include "Atom/Renderer/Framebuffer.h"
#include "Atom/Renderer/Buffer.h"
#include "Atom/Renderer/DescriptorHeap.h"
#include "Atom/Renderer/ResourceStateTracker.h"
#include "Atom/Renderer/Renderer.h"
#include "Atom/Renderer/Camera.h"
#include "Atom/Renderer/EditorCamera.h"
#include "Atom/Renderer/Material.h"
#include "Atom/Renderer/Mesh.h"
#include "Atom/Renderer/ShaderLibrary.h"
#include "Atom/Renderer/PipelineLibrary.h"
#include "Atom/Renderer/LightEnvironment.h"

// Scene
#include "Atom/Scene/Components.h"
#include "Atom/Scene/Entity.h"
#include "Atom/Scene/Scene.h"
#include "Atom/Scene/SceneRenderer.h"