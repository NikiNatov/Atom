#include "atompch.h"
#include "ScriptEngine.h"
#include "Atom/Scene/Components.h"
#include "Atom/Scene/Scene.h"

#include "Atom/Scripting/ScriptEmbeddedModule.h"
#include "Atom/Core/Application.h"

namespace Atom
{
    static HashMap<String, ScriptVariableType> s_ScriptVariableTypes = {
        { "<class 'int'>",              ScriptVariableType::Int         },
        { "<class 'float'>",            ScriptVariableType::Float       },
        { "<class 'bool'>",             ScriptVariableType::Bool        },
        { "<class 'Atom.Vec2'>",        ScriptVariableType::Vec2        },
        { "<class 'Atom.Vec3'>",        ScriptVariableType::Vec3        },
        { "<class 'Atom.Vec4'>",        ScriptVariableType::Vec4        },
        { "<class 'Atom.Entity'>",      ScriptVariableType::Entity      },
        { "<class 'Atom.Material'>",    ScriptVariableType::Material    },
        { "<class 'Atom.Mesh'>",        ScriptVariableType::Mesh        },
        { "<class 'Atom.Texture2D'>",   ScriptVariableType::Texture2D   },
        { "<class 'Atom.TextureCube'>", ScriptVariableType::TextureCube },
    };

    // -----------------------------------------------------ScriptEngine------------------------------------------------------------
    // -----------------------------------------------------------------------------------------------------------------------------
    void ScriptEngine::Initialize(const std::filesystem::path& scriptsDirectory)
    {
        try
        {
            Shutdown();
            ms_AppScriptsDirectory = std::filesystem::canonical(scriptsDirectory);

            py::initialize_interpreter(true, 0, nullptr, false);
            py::module::import("sys").attr("path").attr("append")(ms_AppScriptsDirectory.c_str());
            py::module::import("multiprocessing").attr("set_executable")(py::module::import("os").attr("path").attr("join")(py::module::import("sys").attr("exec_prefix"), "pythonw.exe"));

            LoadScriptModules();
            LoadScriptClasses();

            ms_FileWatcher = CreateScope<filewatch::FileWatch<std::filesystem::path>>(ms_AppScriptsDirectory, [](const std::filesystem::path& path, const filewatch::Event changeType)
            {
                if (changeType == filewatch::Event::added)
                {
                    Application::Get().SubmitForMainThreadExecution([=]()
                    {
                        ScriptEngine::LoadScriptModule(path);
                        ScriptEngine::LoadScriptClasses();
                    });
                }
                else if (changeType == filewatch::Event::modified && !ms_PendingScriptReload)
                {
                    ms_PendingScriptReload = true;

                    using namespace std::literals;
                    std::this_thread::sleep_for(1000ms);

                    Application::Get().SubmitForMainThreadExecution([]()
                    {
                        ScriptEngine::ReloadScriptModules();
                        ScriptEngine::LoadScriptClasses();
                    });
                }
                else if (changeType == filewatch::Event::removed)
                {
                    Application::Get().SubmitForMainThreadExecution([=]()
                    {
                        ScriptEngine::UnloadScriptModule(path.stem().string());
                    });
                }
            });
        }
        catch (py::error_already_set& e)
        {
            ATOM_ERROR(e.what());
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ScriptEngine::Shutdown()
    {
        ms_RunningScene = nullptr;
        ms_ScriptCoreModule.release();
        ms_EntityClass = py::none();
        ms_ScriptClasses.clear();
        ms_ScriptInstances.clear();
        ms_ScriptVariableMaps.clear();
        ms_AppScriptModules.clear();

        if (Py_IsInitialized() != 0)
            py::finalize_interpreter();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ScriptEngine::OnSceneStart(Scene* scene)
    {
        ms_RunningScene = scene;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ScriptEngine::OnSceneStop()
    {
        ms_RunningScene = nullptr;
        ms_ScriptInstances.clear();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ScriptEngine::OnEvent(Event& event)
    {
        for (auto& [uuid, scriptInstance] : ms_ScriptInstances)
        {
            scriptInstance->OnEvent(event);
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ScriptEngine::CreateEntityScript(Entity entity)
    {
        auto& sc = entity.GetComponent<ScriptComponent>();

        if (Ref<ScriptClass> scriptClass = GetScriptClass(sc.ScriptClass))
        {
            Ref<ScriptInstance> scriptInstance = CreateRef<ScriptInstance>(scriptClass, entity);
            ms_ScriptInstances[entity.GetUUID()] = scriptInstance;

            // Set the member variable values from the variable map for the current entity
            auto mapIt = ms_ScriptVariableMaps.find(entity.GetUUID());
            if (mapIt != ms_ScriptVariableMaps.end())
            {
                for (auto& [name, variable] : mapIt->second)
                {
                    switch (variable.GetType())
                    {
                        case ScriptVariableType::Int:         scriptInstance->SetMemberValue(name, variable.GetValue<s32>()); break;
                        case ScriptVariableType::Float:       scriptInstance->SetMemberValue(name, variable.GetValue<f32>()); break;
                        case ScriptVariableType::Bool:        scriptInstance->SetMemberValue(name, variable.GetValue<bool>()); break;
                        case ScriptVariableType::Vec2:        scriptInstance->SetMemberValue(name, variable.GetValue<glm::vec2>()); break;
                        case ScriptVariableType::Vec3:        scriptInstance->SetMemberValue(name, variable.GetValue<glm::vec3>()); break;
                        case ScriptVariableType::Vec4:        scriptInstance->SetMemberValue(name, variable.GetValue<glm::vec4>()); break;
                        case ScriptVariableType::Entity:      scriptInstance->SetMemberValue(name, ScriptWrappers::Entity(variable.GetValue<UUID>())); break;
                        case ScriptVariableType::Material:    scriptInstance->SetMemberValue(name, ScriptWrappers::Material(variable.GetValue<UUID>())); break;
                        case ScriptVariableType::Mesh:        scriptInstance->SetMemberValue(name, ScriptWrappers::Mesh(variable.GetValue<UUID>())); break;
                        case ScriptVariableType::Texture2D:   scriptInstance->SetMemberValue(name, ScriptWrappers::Texture2D(variable.GetValue<UUID>())); break;
                        case ScriptVariableType::TextureCube: scriptInstance->SetMemberValue(name, ScriptWrappers::TextureCube(variable.GetValue<UUID>())); break;
                    }
                }
            }

            scriptInstance->OnCreate();
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ScriptEngine::UpdateEntityScript(Entity entity, Timestep ts)
    {
        if (Ref<ScriptInstance> instance = GetScriptInstance(entity))
        {
            instance->OnUpdate(ts);
        }
        else
        {
            ATOM_ERROR("Entity with ID {} has no script instantiated", entity.GetUUID());
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ScriptEngine::LateUpdateEntityScript(Entity entity, Timestep ts)
    {
        if (Ref<ScriptInstance> instance = GetScriptInstance(entity))
        {
            instance->OnLateUpdate(ts);
        }
        else
        {
            ATOM_ERROR("Entity with ID {} has no script instantiated", entity.GetUUID());
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ScriptEngine::DestroyEntityScript(Entity entity)
    {
        if (Ref<ScriptInstance> instance = GetScriptInstance(entity))
        {
            instance->OnDestroy();
            ms_ScriptInstances.erase(entity.GetUUID());
        }
        else
        {
            ATOM_ERROR("Entity with ID {} has no script instantiated", entity.GetUUID());
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Scene* ScriptEngine::GetRunningScene()
    {
        return ms_RunningScene;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const HashMap<String, Ref<ScriptClass>>& ScriptEngine::GetScriptClasses()
    {
        return ms_ScriptClasses;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<ScriptClass> ScriptEngine::GetScriptClass(const String& className)
    {
        auto it = ms_ScriptClasses.find(className);

        if (it == ms_ScriptClasses.end())
            return nullptr;

        return it->second;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<ScriptInstance> ScriptEngine::GetScriptInstance(Entity entity)
    {
        ATOM_ENGINE_ASSERT(entity);
        return GetScriptInstance(entity.GetUUID());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<ScriptInstance> ScriptEngine::GetScriptInstance(UUID uuid)
    {
        auto it = ms_ScriptInstances.find(uuid);

        if (it == ms_ScriptInstances.end())
            return nullptr;

        return it->second;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    ScriptVariableMap& ScriptEngine::GetScriptVariableMap(Entity entity)
    {
        ATOM_ENGINE_ASSERT(entity);
        return ms_ScriptVariableMaps[entity.GetUUID()];
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ScriptEngine::LoadScriptClasses()
    {
        try
        {
            ms_ScriptClasses.clear();

            // Get all classes that inherint from Entity
            for (auto& cls : ms_EntityClass.attr("__subclasses__")().cast<py::list>())
            {
                Ref<ScriptClass> scriptClass = CreateRef<ScriptClass>(cls.cast<py::object>());
                ms_ScriptClasses[scriptClass->GetName()] = scriptClass;
            }
        }
        catch (py::error_already_set& e)
        {
            ATOM_ERROR(e.what());
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ScriptEngine::LoadScriptModules()
    {
        try
        {
            ms_ScriptCoreModule = py::module::import("Atom");
            ms_EntityClass = ms_ScriptCoreModule.attr("Entity");
            ms_AppScriptModules.clear();

            for (const auto& entry : std::filesystem::directory_iterator(ms_AppScriptsDirectory))
            {
                LoadScriptModule(entry.path());
            }
        }
        catch (py::error_already_set& e)
        {
            ATOM_ERROR(e.what());
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ScriptEngine::LoadScriptModule(const std::filesystem::path& filepath)
    {
        try
        {
            if (filepath.extension() == ".py")
            {
                ms_AppScriptModules.push_back(py::module::import(filepath.stem().string().c_str()));
                ATOM_INFO("Script module \"{}\" loaded", filepath.stem().string());
            }
        }
        catch (py::error_already_set& e)
        {
            ATOM_ERROR(e.what());
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ScriptEngine::UnloadScriptModule(const String& moduleName)
    {
        try
        {
            for (auto it = ms_AppScriptModules.begin(); it != ms_AppScriptModules.end(); it++)
            {
                py::module& module = *it;

                if (module.attr("__name__").cast<String>() == moduleName)
                {
                    py::module inspectModule = py::module::import("inspect");

                    // Remove all classes from that module
                    for (auto& [name, obj] : inspectModule.attr("getmembers")(module, inspectModule.attr("isclass")).cast<py::dict>())
                    {
                        ms_ScriptClasses.erase(name.cast<String>());
                    }

                    module.release();
                    ms_AppScriptModules.erase(it);

                    // Reload the core module so that it is up to date 
                    ms_ScriptCoreModule.reload();

                    ATOM_INFO("Script module \"{}\" unloaded", moduleName);
                    return;
                }
            }
        }
        catch (py::error_already_set& e)
        {
            ATOM_ERROR(e.what());
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ScriptEngine::ReloadScriptModules()
    {
        try
        {
            ms_ScriptCoreModule.reload();

            for (auto& script : ms_AppScriptModules)
                script.reload();

            ms_EntityClass = ms_ScriptCoreModule.attr("Entity");

            ms_PendingScriptReload = false;
            ATOM_INFO("Scripts reloaded");
        }
        catch (py::error_already_set& e)
        {
            ATOM_ERROR(e.what());
        }
    }

    // -----------------------------------------------------ScriptClass-------------------------------------------------------------
    // -----------------------------------------------------------------------------------------------------------------------------
    ScriptClass::ScriptClass(const py::object& pythonClass)
        : m_PythonClass(pythonClass)
    {
        try
        {
            m_Name = m_PythonClass.attr("__name__").cast<String>();

            py::dict classMembersTypeHints = m_PythonClass.attr("__annotations__").cast<py::dict>();

            // Create a temp instance so that we can get all member variables
            py::object tmpInstance = CreateInstance(0);

            for (auto& [name, value] : tmpInstance.attr("__dict__").cast<py::dict>())
            {
                String varName = name.cast<String>();

                // Variables starting with _ we mark as private and don't include them in the fields
                if (varName[0] != '_')
                {
                    String pythonType = classMembersTypeHints.attr("get")(varName).cast<py::str>();

                    ATOM_ENGINE_ASSERT(s_ScriptVariableTypes.find(pythonType) != s_ScriptVariableTypes.end(), "Unsupported type");
                    ScriptVariableType varType = s_ScriptVariableTypes.at(pythonType);

                    ScriptVariable scriptVar = ScriptVariable(varName, varType);

                    switch (varType)
                    {
                        case ScriptVariableType::Int:         scriptVar.SetValue<s32>(value.cast<s32>()); break;
                        case ScriptVariableType::Float:       scriptVar.SetValue<f32>(value.cast<f32>()); break;
                        case ScriptVariableType::Bool:        scriptVar.SetValue<bool>(value.cast<bool>()); break;
                        case ScriptVariableType::Vec2:        scriptVar.SetValue<glm::vec2>(value.cast<glm::vec2>()); break;
                        case ScriptVariableType::Vec3:        scriptVar.SetValue<glm::vec3>(value.cast<glm::vec3>()); break;
                        case ScriptVariableType::Vec4:        scriptVar.SetValue<glm::vec4>(value.cast<glm::vec4>()); break;
                        case ScriptVariableType::Entity:      scriptVar.SetValue<wrappers::Entity>(value.cast<wrappers::Entity>()); break;
                        case ScriptVariableType::Material:    scriptVar.SetValue<UUID>(value.cast<wrappers::Material>().GetUUID()); break;
                        case ScriptVariableType::Mesh:        scriptVar.SetValue<UUID>(value.cast<wrappers::Mesh>().GetUUID()); break;
                        case ScriptVariableType::Texture2D:   scriptVar.SetValue<UUID>(value.cast<wrappers::Texture2D>().GetUUID()); break;
                        case ScriptVariableType::TextureCube: scriptVar.SetValue<UUID>(value.cast<wrappers::TextureCube>().GetUUID()); break;
                    }

                    m_MemberVariables[varName] = scriptVar;
                }
            }
        }
        catch (std::exception& e)
        {
            ATOM_ERROR(e.what());
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    py::object ScriptClass::CreateInstance(UUID entityID)
    {
        return m_PythonClass((u64)entityID);
    }

    // ---------------------------------------------------ScriptInstance------------------------------------------------------------
    // -----------------------------------------------------------------------------------------------------------------------------
    ScriptInstance::ScriptInstance(Ref<ScriptClass> scriptClass, Entity entity)
        : m_Class(scriptClass)
    {
        try
        {
            m_PythonInstance = m_Class->CreateInstance(entity.GetUUID());

            if (py::hasattr(m_PythonInstance, "on_create"))
                m_OnCreateFn = m_PythonInstance.attr("on_create");
            if (py::hasattr(m_PythonInstance, "on_update"))
                m_OnUpdateFn = m_PythonInstance.attr("on_update");
            if (py::hasattr(m_PythonInstance, "on_late_update"))
                m_OnLateUpdateFn = m_PythonInstance.attr("on_late_update");
            if (py::hasattr(m_PythonInstance, "on_destroy"))
                m_OnDestroyFn = m_PythonInstance.attr("on_destroy");
            if (py::hasattr(m_PythonInstance, "on_event"))
                m_OnEventFn = m_PythonInstance.attr("on_event");
        }
        catch (std::exception& e)
        {
            ATOM_ERROR(e.what());
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ScriptInstance::OnCreate()
    {
        try
        {
            if (!m_OnCreateFn.is_none())
                m_OnCreateFn();
        }
        catch (std::exception& e)
        {
            ATOM_ERROR(e.what());
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ScriptInstance::OnUpdate(Timestep ts)
    {
        try
        {
            if (!m_OnUpdateFn.is_none())
                m_OnUpdateFn(ts);
        }
        catch (std::exception& e)
        {
            ATOM_ERROR(e.what());
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ScriptInstance::OnLateUpdate(Timestep ts)
    {
        try
        {
            if (!m_OnLateUpdateFn.is_none())
                m_OnLateUpdateFn(ts);
        }
        catch (std::exception& e)
        {
            ATOM_ERROR(e.what());
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ScriptInstance::OnDestroy()
    {
        try
        {
            if (!m_OnDestroyFn.is_none())
                m_OnDestroyFn();
        }
        catch (std::exception& e)
        {
            ATOM_ERROR(e.what());
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ScriptInstance::OnEvent(Event& event)
    {
        try
        {
            if (!m_OnEventFn.is_none())
            {
                EventDispatcher dispatcher(event);
                dispatcher.Dispatch<KeyPressedEvent>([&](KeyPressedEvent& e) { m_OnEventFn(e); return false; });
                dispatcher.Dispatch<KeyReleasedEvent>([&](KeyReleasedEvent& e) { m_OnEventFn(e); return false; });
                dispatcher.Dispatch<MouseButtonPressedEvent>([&](MouseButtonPressedEvent& e) { m_OnEventFn(e); return false; });
                dispatcher.Dispatch<MouseButtonReleasedEvent>([&](MouseButtonReleasedEvent& e) { m_OnEventFn(e); return false; });
                dispatcher.Dispatch<MouseMovedEvent>([&](MouseMovedEvent& e) { m_OnEventFn(e); return false; });
                dispatcher.Dispatch<MouseScrolledEvent>([&](MouseScrolledEvent& e) { m_OnEventFn(e); return false; });
            }
        }
        catch (std::exception& e)
        {
            ATOM_ERROR(e.what());
        }
    }
}