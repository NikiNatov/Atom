#include "atompch.h"
#include "Project.h"

#include "Atom/Project/ProjectSerializer.h"
#include "Atom/Asset/AssetManager.h"
#include "Atom/Scripting/ScriptEngine.h"
#include "Atom/Tools/ContentTools.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<Project> Project::NewProject(const String& name, const String& startSceneName, const std::filesystem::path& projectLocation)
    {
        ms_ActiveProject = CreateRef<Project>();
        ms_ActiveProject->m_ProjectDirectory = std::filesystem::canonical(projectLocation) / name;
        ms_ActiveProject->m_Settings.Name = name;
        ms_ActiveProject->m_Settings.AssetsDirectory = "Assets";
        ms_ActiveProject->m_Settings.ScriptsDirectory = "Scripts";
        ms_ActiveProject->m_Settings.StartScenePath = std::filesystem::path("Scenes") / (startSceneName + ".atmscene");

        std::filesystem::path scriptsDirAbsolutePath = ms_ActiveProject->m_ProjectDirectory / ms_ActiveProject->m_Settings.ScriptsDirectory;
        std::filesystem::path assetsDirAbsolutePath = ms_ActiveProject->m_ProjectDirectory / ms_ActiveProject->m_Settings.AssetsDirectory;

        std::filesystem::create_directories(scriptsDirAbsolutePath);
        std::filesystem::create_directories(assetsDirAbsolutePath);
        std::filesystem::create_directory(assetsDirAbsolutePath / "Textures");
        std::filesystem::create_directory(assetsDirAbsolutePath / "Materials");
        std::filesystem::create_directory(assetsDirAbsolutePath / "Scenes");
        std::filesystem::create_directory(assetsDirAbsolutePath / "Meshes");

        SaveActiveProject();

        AssetManager::Initialize(assetsDirAbsolutePath);
        ScriptEngine::Initialize(scriptsDirAbsolutePath);

        ContentTools::CreateSceneAsset(startSceneName, ms_ActiveProject->m_Settings.StartScenePath);

        return ms_ActiveProject;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<Project> Project::OpenProject(const std::filesystem::path& filepath)
    {
        Ref<Project> newProject = CreateRef<Project>();
        ProjectSerializer serializer(newProject);

        if (!serializer.Deserialize(filepath))
            return nullptr;

        newProject->m_ProjectDirectory = filepath.parent_path();
        ms_ActiveProject = newProject;

        AssetManager::Initialize(ms_ActiveProject->m_ProjectDirectory / ms_ActiveProject->m_Settings.AssetsDirectory);
        ScriptEngine::Initialize(ms_ActiveProject->m_ProjectDirectory / ms_ActiveProject->m_Settings.ScriptsDirectory);

        return ms_ActiveProject;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool Project::SaveActiveProject()
    {
        String projectFilename = ms_ActiveProject->m_Settings.Name + ".atmproj";
        ProjectSerializer serializer(ms_ActiveProject);

        if (!serializer.Serialize(ms_ActiveProject->m_ProjectDirectory / projectFilename))
            return false;

        return true;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<Project> Project::GetActiveProject()
    {
        return ms_ActiveProject;
    }
}
