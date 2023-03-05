#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Asset/Asset.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Atom
{
    class Animation : public Asset
    {
        friend class AssetSerializer;
        friend class ContentTools;
    public:
        struct BoneTransform
        {
            glm::vec3 Position;
            glm::quat Rotation;
            glm::vec3 Scale = glm::vec3(1.0);
        };

        struct KeyFrame
        {
            f32 TimeStamp;
            HashMap<u32, BoneTransform> BoneTransforms;

            inline bool operator<(const KeyFrame& rhs) const { return TimeStamp < rhs.TimeStamp; }
            inline bool operator==(const KeyFrame& rhs) const { return TimeStamp == rhs.TimeStamp; }
        };

    public:
        Animation(f32 duration, f32 ticksPerSecond, const Set<KeyFrame>& keyFrames);

        glm::mat4 GetTransformAtTimeStamp(f32 timeStamp, u32 boneID);

        inline f32 GetDuration() const { return m_Duration; }
        inline f32 GetTicksPerSecond() const { return m_TicksPerSecond; }
        inline const Set<KeyFrame>& GetKeyFrames() const { return m_KeyFrames; }
    private:
        f32           m_Duration;
        f32           m_TicksPerSecond;
        Set<KeyFrame> m_KeyFrames;
    };

    class Skeleton : public Asset
    {
        friend class AssetSerializer;
        friend class ContentTools;
    public:
        struct Bone
        {
            static constexpr u32 MAX_BONE_WEIGHTS = 4;

            u32 ID = UINT32_MAX;
            u32 ParentID = UINT32_MAX;
            Vector<u32> ChildrenIDs;
            glm::mat4 InverseBindTransform = glm::mat4(1.0f);
            glm::mat4 AnimatedTransform = glm::mat4(1.0f);

        };
    public:
        Skeleton(const Vector<Bone>& bones);

        inline Bone& GetRootBone() { return m_Bones[m_RootBoneID]; }
        inline Vector<Bone>& GetBones() { return m_Bones; }
    private:
        u32          m_RootBoneID;
        Vector<Bone> m_Bones;
    };
}