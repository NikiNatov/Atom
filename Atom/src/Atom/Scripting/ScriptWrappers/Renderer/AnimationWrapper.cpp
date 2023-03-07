#include "atompch.h"
#include "AnimationWrapper.h"

#include "Atom/Asset/AssetManager.h"

namespace Atom
{
    namespace ScriptWrappers
    {
        // -----------------------------------------------------------------------------------------------------------------------------
        Animation::Animation(u64 assetUUID)
            : m_Animation(nullptr)
        {
            if (assetUUID != 0)
                m_Animation = AssetManager::GetAsset<Atom::Animation>(assetUUID, true);
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        Animation::Animation(const Ref<Atom::Animation>& animation)
            : m_Animation(animation)
        {
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        f32 Animation::GetDuration() const
        {
            if (m_Animation)
                return m_Animation->GetDuration();

            ATOM_ERROR("Animation::GetDuration called on a NULL asset");
            return 0.0f;
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        f32 Animation::GetTicksPerSecond() const
        {
            if (m_Animation)
                return m_Animation->GetTicksPerSecond();

            ATOM_ERROR("Animation::GetTicksPerSecond called on a NULL asset");
            return 0.0f;
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        UUID Animation::GetUUID() const
        {
            return m_Animation ? m_Animation->GetUUID() : 0;
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        Ref<Atom::Animation> Animation::GetAnimation() const
        {
            return m_Animation;
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        Animation Animation::Find(const std::filesystem::path& assetPath)
        {
            UUID uuid = AssetManager::GetUUIDForAssetPath(AssetManager::GetAssetsFolder() / assetPath);

            if (uuid == 0)
                return Animation(nullptr);

            return Animation(AssetManager::GetAsset<Atom::Animation>(uuid, true));
        }
    }
}