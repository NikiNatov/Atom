#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Renderer/Animation.h"

#include <pybind11/pybind11.h>

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

        class AnimationController
        {
        public:
            AnimationController(u64 assetUUID);
            AnimationController(const Ref<Atom::AnimationController>& controller);

            void TransitionToState(u16 state);

            Animation GetInitialState() const;
            Animation GetCurrentState() const;
            pybind11::list GetAnimationStates() const;

            UUID GetUUID() const;
            Ref<Atom::AnimationController> GetAnimationController() const;

            static AnimationController Find(const std::filesystem::path& assetPath);
        private:
            Ref<Atom::AnimationController> m_AnimationController = nullptr;
        };
    }
}