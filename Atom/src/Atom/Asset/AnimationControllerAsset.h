#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Asset/Asset.h"
#include "Atom/Asset/AnimationAsset.h"

namespace Atom
{
    class AnimationController : public Asset
    {
        friend class AssetSerializer;
        friend class ContentTools;
    public:
        AnimationController(const Vector<Ref<Animation>>& animationStates, u16 initialStateIdx);

        void TransitionToState(u16 newState);

        inline Ref<Animation> GetInitialState() const { return m_AnimationStates[m_InitialStateIdx]; }
        inline Ref<Animation> GetCurrentState() const { return m_AnimationStates[m_CurrentStateIdx]; }
        inline const Vector<Ref<Animation>>& GetAnimationStates() const { return m_AnimationStates; }
    private:
        u32                    m_InitialStateIdx;
        u32                    m_CurrentStateIdx;
        Vector<Ref<Animation>> m_AnimationStates;
    };
}