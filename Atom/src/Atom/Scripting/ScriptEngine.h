#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Core/Timestep.h"
#include "Atom/Scene/Entity.h"

#include <pybind11/pybind11.h>
#include <pybind11/embed.h>

namespace Atom
{
    enum class ScriptVariableType
    {
        None = 0,
        Int,
        Float,
        Bool,
        Vec2, Vec3, Vec4,
        Entity
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
        void OnDestroy();

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
        inline const pybind11::object& GetPythonInstance() const { return m_PythonInstance; }
    private:
        Ref<ScriptClass> m_Class;
        pybind11::object m_PythonInstance;
        pybind11::object m_OnCreateFn = pybind11::none();
        pybind11::object m_OnUpdateFn = pybind11::none();
        pybind11::object m_OnDestroyFn = pybind11::none();
    };

    class ScriptEngine
    {
    public:
        static void Initialize();
        static void Shutdown();

        static void OnSceneStart(Scene* scene);
        static void OnSceneStop();

        static void CreateEntityScript(Entity entity);
        static void UpdateEntityScript(Entity entity, Timestep ts);

        static Scene* GetRunningScene();
        static const HashMap<String, Ref<ScriptClass>>& GetScriptClasses();
        static Ref<ScriptClass> GetScriptClass(const String& className);
        static Ref<ScriptInstance> GetScriptInstance(Entity entity);
        static ScriptVariableMap& GetScriptVariableMap(Entity entity);
    private:
        static void LoadScriptClasses();
    private:
        inline static Scene*                             ms_RunningScene = nullptr;
        inline static HashMap<String, Ref<ScriptClass>>  ms_ScriptClasses;
        inline static HashMap<UUID, Ref<ScriptInstance>> ms_ScriptInstances;
        inline static HashMap<UUID, ScriptVariableMap>   ms_ScriptVariableMaps;
        inline static std::filesystem::path              ms_ScriptsDirectory;
        inline static pybind11::object                   ms_EntityClass;
        inline static pybind11::object                   ms_ScriptCoreModule;
    };
}