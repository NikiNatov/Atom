#include "atompch.h"
#include "AnimationControllerAsset.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    AnimationController::AnimationController(const Vector<Ref<Animation>>& animationStates, u16 initialStateIdx)
        : Asset(AssetType::AnimationController), m_AnimationStates(animationStates), m_InitialStateIdx(initialStateIdx), m_CurrentStateIdx(initialStateIdx)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void AnimationController::TransitionToState(u16 newState)
    {
        m_CurrentStateIdx = newState;
    }
}