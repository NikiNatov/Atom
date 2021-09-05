#pragma once

#include "Core.h"
#include "Layer.h"

namespace Atom
{
    class LayerStack
    {
    public:
        LayerStack() = default;
        ~LayerStack();

        void PushLayer(Layer* layer);
        void PopLayer(Layer* layer);
        void PushOverlay(Layer* layer);
        void PopOverlay(Layer* layer);

        Vector<Layer*>::iterator begin() { return m_Layers.begin(); }
        Vector<Layer*>::iterator end() { return m_Layers.end(); }
        Vector<Layer*>::const_iterator cbegin() const { return m_Layers.cbegin(); }
        Vector<Layer*>::const_iterator cend() const { return m_Layers.cend(); }
    private:
        Vector<Layer*>  m_Layers;
        u32             m_InsertIndex = 0;
    };
}