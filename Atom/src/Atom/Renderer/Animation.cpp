#include "atompch.h"
#include "Animation.h"

#include <glm\gtx\quaternion.hpp>

namespace Atom
{
    // ---------------------------------------------------------- Animation --------------------------------------------------------
    // -----------------------------------------------------------------------------------------------------------------------------
    Animation::Animation(f32 duration, f32 ticksPerSecond, const Set<KeyFrame>& keyFrames)
        : Asset(AssetType::Animation), m_Duration(duration), m_TicksPerSecond(ticksPerSecond), m_KeyFrames(keyFrames)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    glm::mat4 Animation::GetTransformAtTimeStamp(f32 timeStamp, u32 boneID)
    {
        const KeyFrame* prevKeyFrame = nullptr;
        const KeyFrame* nextKeyFrame = nullptr;

        for (auto& keyFrame : m_KeyFrames)
        {
            if (keyFrame.TimeStamp > timeStamp)
                break;

            if (keyFrame.BoneTransforms.find(boneID) != keyFrame.BoneTransforms.end())
                prevKeyFrame = &keyFrame;
        }

        for (auto& keyFrame : m_KeyFrames)
        {
            if (keyFrame.TimeStamp > timeStamp && keyFrame.BoneTransforms.find(boneID) != keyFrame.BoneTransforms.end())
            {
                nextKeyFrame = &keyFrame;
                break;
            }
        }

        ATOM_ENGINE_ASSERT(prevKeyFrame || nextKeyFrame);
        
        BoneTransform interpolatedTransform;
        if (prevKeyFrame && !nextKeyFrame)
        {
            interpolatedTransform.Position = prevKeyFrame->BoneTransforms.at(boneID).Position;
            interpolatedTransform.Rotation = prevKeyFrame->BoneTransforms.at(boneID).Rotation;
            interpolatedTransform.Scale = prevKeyFrame->BoneTransforms.at(boneID).Scale;
        }
        else if (!prevKeyFrame && nextKeyFrame)
        {
            interpolatedTransform.Position = nextKeyFrame->BoneTransforms.at(boneID).Position;
            interpolatedTransform.Rotation = nextKeyFrame->BoneTransforms.at(boneID).Rotation;
            interpolatedTransform.Scale = nextKeyFrame->BoneTransforms.at(boneID).Scale;
        }
        else
        {
            f32 interpolationFactor = (timeStamp - prevKeyFrame->TimeStamp) / (nextKeyFrame->TimeStamp - prevKeyFrame->TimeStamp);
            interpolatedTransform.Position = glm::mix(prevKeyFrame->BoneTransforms.at(boneID).Position, nextKeyFrame->BoneTransforms.at(boneID).Position, interpolationFactor);
            interpolatedTransform.Rotation = glm::slerp(prevKeyFrame->BoneTransforms.at(boneID).Rotation, nextKeyFrame->BoneTransforms.at(boneID).Rotation, interpolationFactor);
            interpolatedTransform.Scale = glm::mix(prevKeyFrame->BoneTransforms.at(boneID).Scale, nextKeyFrame->BoneTransforms.at(boneID).Scale, interpolationFactor);
        }
        
        return glm::translate(glm::mat4(1.0f), interpolatedTransform.Position) * glm::toMat4(interpolatedTransform.Rotation) * glm::scale(glm::mat4(1.0f), interpolatedTransform.Scale);
    }

    // ----------------------------------------------------------- Skeleton --------------------------------------------------------
    // -----------------------------------------------------------------------------------------------------------------------------
    Skeleton::Skeleton(const Vector<Bone>& bones)
        : Asset(AssetType::Skeleton), m_Bones(bones), m_RootBoneID(UINT32_MAX)
    {
        for (auto& bone : bones)
        {
            if (bone.ParentID == UINT32_MAX)
            {
                m_RootBoneID = bone.ID;
                break;
            }
        }

        ATOM_ENGINE_ASSERT(m_RootBoneID != UINT32_MAX);
    }

    // ------------------------------------------------------ AnimationController --------------------------------------------------
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
