#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Core/UUID.h"
#include "Atom/Scene/Components.h"
#include "Atom/Scene/Scene.h"
#include "Atom/Scripting/ScriptEngine.h"

namespace Atom 
{
    namespace ScriptWrappers
    {
        class Entity
        {
        public:
            Entity();
            Entity(u64 uuid);

            void SetTag(const String& tag);
            void SetTranslation(const glm::vec3& translation);
            void SetRotation(const glm::vec3& rotation);
            void SetScale(const glm::vec3& scale);

            u64 GetUUID() const;
            const String& GetTag() const;
            pybind11::object GetScriptInstance() const;

            template<typename T>
            T AddComponent();

            template<typename T>
            bool HasComponent() const;

            template<typename T>
            T GetComponent() const;

            inline bool IsValid() const { return m_UUID != 0; }
        public:
            static Entity FindEntityByName(const String& name);
            static Entity CreateEntity(const String& name);

        private:
            u64 m_UUID;
        };
    }
}