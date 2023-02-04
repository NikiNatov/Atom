#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Core/Timestep.h"
#include "Atom/Core/Events/Events.h"
#include "Atom/Scene/Entity.h"

#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <FileWatch.h>

namespace Atom
{
    enum class ScriptVariableType
    {
        None = 0,
        Int,
        Float,
        Bool,
        Vec2, Vec3, Vec4,
        Entity,
        Material,
        Mesh,
        Texture2D,
        TextureCube
    };

    struct ScriptVariable
    {
    public:
        ScriptVariable() = default;
        ScriptVariable(const String& name, ScriptVariableType type)
            : m_Name(name), m_Type(type) {}

        inline const String& GetName() const { return m_Name; }
        inline ScriptVariableType GetType() const { return m_Type; }

        template<typename ValueType>
        void SetValue(const ValueType& value)
        {
            static_assert(sizeof(ValueType) <= 16, "Value type is too large");
            memcpy(m_InitialDataBuffer, &value, sizeof(ValueType));
        }

        template<typename ValueType>
        ValueType GetValue()
        {
            static_assert(sizeof(ValueType) <= 16, "Value type is too large");
            return *(ValueType*)m_InitialDataBuffer;
        }

    private:
        String             m_Name = "";
        ScriptVariableType m_Type = ScriptVariableType::None;
        byte               m_InitialDataBuffer[16];
    };

    using ScriptVariableMap = HashMap<String, ScriptVariable>;

    class ScriptClass
    {
    public:
        ScriptClass(const pybind11::object& pythonClass);

        pybind11::object CreateInstance(UUID entityID);

        inline const String& GetName() const { return m_Name; }
        inline const ScriptVariableMap& GetMemberVariables() const { return m_MemberVariables; }
    private:
        String            m_Name;
        ScriptVariableMap m_MemberVariables;
        pybind11::object  m_PythonClass;
    };

    class ScriptInstance
    {
    public:
        ScriptInstance(Ref<ScriptClass> scriptClass, Entity entity);

        void OnCreate();
        void OnUpdate(Timestep ts);
        void OnLateUpdate(Timestep ts);
        void OnDestroy();
        void OnEvent(Event& event);

        template<typename ValueType>
        void SetMemberValue(const String& name, const ValueType& value)
        {
            ATOM_ENGINE_ASSERT(m_Class->GetMemberVariables().find(name) != m_Class->GetMemberVariables().end(), "Member variable does not exist in class");
            m_PythonInstance.attr(name.c_str()) = value;
        }

        template<typename ValueType>
        ValueType GetMemberValue(const String& name)
        {
            ATOM_ENGINE_ASSERT(m_Class->GetMemberVariables().find(name) != m_Class->GetMemberVariables().end(), "Member variable does not exist in class");
            return m_PythonInstance.attr(name.c_str()).cast<ValueType>();
        }

        inline Ref<ScriptClass> GetClass() const { return m_Class; }
        inline pybind11::object GetPythonInstance() const { return m_PythonInstance; }
    private:
        Ref<ScriptClass> m_Class;
        pybind11::object m_PythonInstance;
        pybind11::object m_OnCreateFn = pybind11::none();
        pybind11::object m_OnUpdateFn = pybind11::none();
        pybind11::object m_OnLateUpdateFn = pybind11::none();
        pybind11::object m_OnDestroyFn = pybind11::none();
        pybind11::object m_OnEventFn = pybind11::none();
    };

    class ScriptEngine
    {
    public:
        static void Initialize(const std::filesystem::path& scriptsDirectory);
        static void Shutdown();

        static void OnSceneStart(Scene* scene);
        static void OnSceneStop();
        static void OnEvent(Event& event);

        static void CreateEntityScript(Entity entity);
        static void UpdateEntityScript(Entity entity, Timestep ts);
        static void LateUpdateEntityScript(Entity entity, Timestep ts);
        static void DestroyEntityScript(Entity entity);

        static Scene* GetRunningScene();
        static const HashMap<String, Ref<ScriptClass>>& GetScriptClasses();
        static Ref<ScriptClass> GetScriptClass(const String& className);
        static Ref<ScriptInstance> GetScriptInstance(Entity entity);
        static Ref<ScriptInstance> GetScriptInstance(UUID uuid);
        static ScriptVariableMap& GetScriptVariableMap(Entity entity);
    private:
        static void LoadScriptClasses();
        static void LoadScriptModules();
        static void LoadScriptModule(const std::filesystem::path& filepath);
        static void UnloadScriptModule(const String& moduleName);
        static void ReloadScriptModules();
    private:
        inline static Scene*                                             ms_RunningScene = nullptr;
        inline static HashMap<String, Ref<ScriptClass>>                  ms_ScriptClasses;
        inline static HashMap<UUID, Ref<ScriptInstance>>                 ms_ScriptInstances;
        inline static HashMap<UUID, ScriptVariableMap>                   ms_ScriptVariableMaps;
        inline static std::filesystem::path                              ms_AppScriptsDirectory;
        inline static pybind11::object                                   ms_EntityClass;
        inline static pybind11::module                                   ms_ScriptCoreModule;
        inline static Vector<pybind11::module>                           ms_AppScriptModules;
        inline static Scope<filewatch::FileWatch<std::filesystem::path>> ms_FileWatcher;
        inline static bool                                               ms_PendingScriptReload = false;
    };
}