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

        private:
            u64 m_UUID;
        };
    }
}