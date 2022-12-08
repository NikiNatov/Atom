#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Scene/Scene.h"

namespace Atom
{
    class SceneSerializer
    {
    public:
        SceneSerializer(const Ref<Scene>& scene);

        void Serialize(const std::filesystem::path& filepath);
        bool Deserialize(const std::filesystem::path& filepath);

        void SetScene(const Ref<Scene>& scene);

    private:
        Ref<Scene> m_Scene;
    };
}