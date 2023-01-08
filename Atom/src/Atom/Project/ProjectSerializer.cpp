#include "atompch.h"
#include "ProjectSerializer.h"

#include <yaml-cpp/yaml.h>

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    ProjectSerializer::ProjectSerializer(const Ref<Project>& project)
        : m_Project(project)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool ProjectSerializer::Serialize(const std::filesystem::path& filepath)
    {
        const ProjectSettings& settings = m_Project->GetSettings();

        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "Project" << YAML::Value;
        {
            out << YAML::BeginMap;
            out << YAML::Key << "Name" << YAML::Value << settings.Name;
            out << YAML::Key << "StartScenePath" << YAML::Value << settings.StartScenePath.string();
            out << YAML::Key << "AssetsDirectory" << YAML::Value << settings.AssetsDirectory.string();
            out << YAML::Key << "ScriptsDirectory" << YAML::Value << settings.ScriptsDirectory.string();
            out << YAML::EndMap;
        }
        out << YAML::EndMap;

        std::ofstream ofs(filepath);
        ofs << out.c_str();

        return true;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool ProjectSerializer::Deserialize(const std::filesystem::path& filepath)
    {
        std::ifstream stream(filepath);
        std::stringstream strStream;

        strStream << stream.rdbuf();

        YAML::Node data = YAML::Load(strStream.str());
        YAML::Node projectNode = data["Project"];

        if (!projectNode)
            return false;

        ProjectSettings& settings = m_Project->GetSettings();
        settings.Name = projectNode["Name"].as<String>();
        settings.StartScenePath = projectNode["StartScenePath"].as<String>();
        settings.AssetsDirectory = projectNode["AssetsDirectory"].as<String>();
        settings.ScriptsDirectory = projectNode["ScriptsDirectory"].as<String>();

        return true;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ProjectSerializer::SetContext(const Ref<Project>& project)
    {
        m_Project = project;
    }
}
