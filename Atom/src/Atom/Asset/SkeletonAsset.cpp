#include "atompch.h"
#include "SkeletonAsset.h"

namespace Atom
{
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
}
