#include "atompch.h"
#include "AnimationAsset.h"

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
}
