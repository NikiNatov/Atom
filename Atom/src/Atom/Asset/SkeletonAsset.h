#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Asset/Asset.h"

#include <glm/glm.hpp>

namespace Atom
{
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