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

            u64 GetUUID() const;
            const String& GetTag() const;
            pybind11::object GetScriptInstance() const;

            template<typename T>
            bool HasComponent() const
            {
                Scene* scene = ScriptEngine::GetRunningScene();
                Atom::Entity entity = scene->FindEntityByUUID(m_UUID);
                return entity.HasComponent<T>();
            }

            template<typename T>
            T& GetComponent() const
            {
                Scene* scene = ScriptEngine::GetRunningScene();
                Atom::Entity entity = scene->FindEntityByUUID(m_UUID);
                return entity.GetComponent<T>();
            }

            inline bool IsValid() const { return m_UUID != 0; }
        public:
            static Entity FindEntityByName(const String& name);

        private:
            u64 m_UUID;
        };
    }
}