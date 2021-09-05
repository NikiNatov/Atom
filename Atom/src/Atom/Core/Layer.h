#pragma once

#include "Core.h"
#include "Timestep.h"
#include "Events/Events.h"

namespace Atom
{
    class Layer
    {
    public:
        Layer(const String& debugName);
        virtual ~Layer() = default;

        virtual void OnAttach() {};
        virtual void OnDetach() {};
        virtual void OnImGuiRender() {};

        // TODO: Add timestep
        virtual void OnUpdate(Timestep ts) {};
        virtual void OnEvent(Event& event) {};
    private:
        String m_DebugName;
    };
}