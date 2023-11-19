#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Renderer/EditorCamera.h"
#include "Atom/Renderer/Renderer.h"
#include "Atom/Scene/Entity.h"
#include "Atom/Asset/Asset.h"

#include <entt/entt.hpp>

namespace Atom
{
    enum class SceneState
    {
        None = 0,
        Edit,
        Running
    };

    class Scene : public Asset
    {
        friend class AssetSerializer;
        friend class SceneSerializer;
        friend class SceneHierarchyPanel;
        friend class Entity;
    public:
        Scene(const String& name = "Unnamed scene");
        ~Scene() = default;

        Scene(const Scene& rhs) = delete;
        Scene& operator=(const Scene& rhs) = delete;

        Scene(Scene&& rhs) noexcept;
        Scene& operator=(Scene&& rhs) noexcept;

        Ref<Scene> Copy();
        Entity CreateEntity(const String& name = "Unnamed Entity");
        Entity CreateEntityFromUUID(UUID uuid, const String& name = "Unnamed Entity");
        void DuplicateEntity(Entity entity);
        void DeleteEntity(Entity entity);
        Entity FindEntityByUUID(UUID uuid);
        Entity FindEntityByName(const String& name);

        void OnStart();
        void OnUpdate(Timestep ts);
        void OnStop();
        void OnEditRender(Ref<Renderer> renderer);
        void OnRuntimeRender(Ref<Renderer> renderer);
        void OnImGuiRender();
        void OnViewportResize(u32 width, u32 height);

        inline const String& GetName() { return m_Name; }
        inline EditorCamera& GetEditorCamera() { return m_EditorCamera; }
        inline SceneState GetSceneState() const { return m_State; }
    private:
        f32                   m_PhysicsUpdateTime = 0.0f;
        String                m_Name;
        entt::registry        m_Registry;
        EditorCamera          m_EditorCamera;
        SceneState            m_State = SceneState::Edit;
        HashMap<UUID, Entity> m_EntitiesByID;
    };
}