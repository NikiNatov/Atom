#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Core/UUID.h"

namespace Atom
{
    struct ProjectSettings
    {
        String                Name = "Untitled Project";
        std::filesystem::path StartScenePath;
        std::filesystem::path AssetsDirectory;
        std::filesystem::path ScriptsDirectory;
    };

    class Project
    {
    public:
        inline ProjectSettings& GetSettings() { return m_Settings; }
        inline const std::filesystem::path& GetProjectDirectory() { return m_ProjectDirectory; }
    public:
        static Ref<Project> NewProject(const String& name, const String& startSceneName, const std::filesystem::path& projectLocation);
        static Ref<Project> OpenProject(const std::filesystem::path& filepath);
        static bool SaveActiveProject();
        static Ref<Project> GetActiveProject();
    private:
        ProjectSettings       m_Settings;
        std::filesystem::path m_ProjectDirectory;
    private:
        inline static Ref<Project> ms_ActiveProject = nullptr;
    };
}