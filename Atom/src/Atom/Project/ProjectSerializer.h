#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Project/Project.h"

namespace Atom
{
    class ProjectSerializer
    {
    public:
        ProjectSerializer(const Ref<Project>& project);

        bool Serialize(const std::filesystem::path& filepath);
        bool Deserialize(const std::filesystem::path& filepath);

        void SetContext(const Ref<Project>& project);
    private:
        Ref<Project> m_Project;
    };
}