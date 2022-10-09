#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Renderer/Pipeline.h"

namespace Atom
{
    class PipelineLibrary
    {
    public:
        PipelineLibrary() = default;
        ~PipelineLibrary() = default;

        template<typename PipelineType>
        void Add(const String& name, const Ref<PipelineType>& pipeline)
        {
            bool exists = Exists(name);
            ATOM_ENGINE_ASSERT(!exists, "Pipeline with that name already exists!");

            if (!exists)
                m_PipelineMap[name] = pipeline;
        }

        template<typename PipelineType, typename PipelineDescType>
        Ref<PipelineType> Load(const String& name, const PipelineDescType& description)
        {
            Ref<PipelineType> pipeline = CreateRef<PipelineType>(description, name.c_str());
            Add(name, pipeline);
            return pipeline;
        }

        template<typename PipelineType>
        Ref<PipelineType> Get(const String& name) const
        {
            ATOM_ENGINE_ASSERT(Exists(name), "Pipeline does not exist!");
            Ref<PipelineType> pipeline = std::dynamic_pointer_cast<PipelineType>(m_PipelineMap.at(name));
            ATOM_ENGINE_ASSERT(pipeline, "Incorrect pipeline type!");
            return pipeline;
        }

        void Clear();
        bool Exists(const String& name) const;
    private:
        HashMap<String, Ref<Pipeline>> m_PipelineMap;
    };
}