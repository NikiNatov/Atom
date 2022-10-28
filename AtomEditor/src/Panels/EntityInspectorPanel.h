#pragma once

#include <Atom.h>

namespace Atom
{
    class EntityInspectorPanel
    {
    public:
        EntityInspectorPanel() = default;
        ~EntityInspectorPanel() = default;

        void OnImGuiRender();
        inline void SetEntity(Entity entity) { m_Entity = entity; }
    private:
        Entity m_Entity = {};
    };
}