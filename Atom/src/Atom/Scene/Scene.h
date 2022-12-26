#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Renderer/EditorCamera.h"
#include "Atom/Scene/Entity.h"
#include "Atom/Scene/SceneRenderer.h"

#include <entt/entt.hpp>

namespace Atom
{
    enum class SceneState
    {
        None = 0,
        Edit,
        Running
    };

    class Scene
    {
        friend class SceneSerializer;
        friend class SceneHierarchyPanel;
        friend class Entity;
    public:
        Scene(const String& name = "Unnamed scene");
        ~Scene() = default;

        Entity CreateEntity(const String& name = "Unnamed Entity");
        Entity CreateEntityFromUUID(UUID uuid, const String& name = "Unnamed Entity");
        void DeleteEntity(Entity entity);
        Entity FindEntityByUUID(UUID uuid);
        Entity FindEntityByName(const String& name);

        void OnStart();
        void OnUpdate(Timestep ts);
        void OnStop();
        void OnEditRender(Ref<SceneRenderer> renderer);
        void OnRuntimeRender(Ref<SceneRenderer> renderer);
        void OnViewportResize(u32 width, u32 height);

        inline void SetSceneState(SceneState state) { m_State = state; }
        inline const String& GetName() { return m_Name; }
        inline EditorCamera& GetEditorCamera() { return m_EditorCamera; }
        inline SceneState GetSceneState() const { return m_State; }
    private:
        String                m_Name;
        entt::registry        m_Registry;
        EditorCamera          m_EditorCamera;
        SceneState            m_State = SceneState::Edit;
        HashMap<UUID, Entity> m_EntitiesByID;
        Ref<LightEnvironment> m_LightEnvironment;
    };
}