#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Renderer/Animation.h"

namespace Atom
{
    namespace ScriptWrappers
    {
        class Animation
        {
        public:
            Animation(u64 assetUUID);
            Animation(const Ref<Atom::Animation>& animation);

            f32 GetDuration() const;
            f32 GetTicksPerSecond() const;

            UUID GetUUID() const;
            Ref<Atom::Animation> GetAnimation() const;

            static Animation Find(const std::filesystem::path& assetPath);
        private:
            Ref<Atom::Animation> m_Animation = nullptr;
        };
    }
}