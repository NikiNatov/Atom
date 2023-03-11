#include "atompch.h"
#include "AnimationWrapper.h"

#include "Atom/Asset/AssetManager.h"

namespace Atom
{
    namespace ScriptWrappers
    {
        // --------------------------------------------------- Animation ---------------------------------------------------------------
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

        // ------------------------------------------------ Animation Controller -------------------------------------------------------
        // -----------------------------------------------------------------------------------------------------------------------------
        AnimationController::AnimationController(u64 assetUUID)
            : m_AnimationController(nullptr)
        {
            if (assetUUID != 0)
                m_AnimationController = AssetManager::GetAsset<Atom::AnimationController>(assetUUID, true);
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        AnimationController::AnimationController(const Ref<Atom::AnimationController>& controller)
            : m_AnimationController(controller)
        {
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        void AnimationController::TransitionToState(u16 state)
        {
            if (m_AnimationController)
                m_AnimationController->TransitionToState(state);
            else
                ATOM_ERROR("AnimationController::TransitionToState called on a NULL asset");
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        Animation AnimationController::GetInitialState() const
        {
            if (m_AnimationController)
                return Animation(m_AnimationController->GetInitialState()->GetUUID());

            ATOM_ERROR("AnimationController::GetInitialState called on a NULL asset");
            return Animation(0);
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        Animation AnimationController::GetCurrentState() const
        {
            if (m_AnimationController)
                return Animation(m_AnimationController->GetCurrentState()->GetUUID());

            ATOM_ERROR("AnimationController::GetCurrentState called on a NULL asset");
            return Animation(0);
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        pybind11::list AnimationController::GetAnimationStates() const
        {
            if (m_AnimationController)
                return pybind11::cast(m_AnimationController->GetAnimationStates());

            ATOM_ERROR("AnimationController::GetAnimationStates called on a NULL asset");
            return pybind11::none();
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        UUID AnimationController::GetUUID() const
        {
            return m_AnimationController ? m_AnimationController->GetUUID() : 0;
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        Ref<Atom::AnimationController> AnimationController::GetAnimationController() const
        {
            return m_AnimationController;
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        AnimationController AnimationController::Find(const std::filesystem::path& assetPath)
        {
            UUID uuid = AssetManager::GetUUIDForAssetPath(AssetManager::GetAssetsFolder() / assetPath);

            if (uuid == 0)
                return AnimationController(nullptr);

            return AnimationController(AssetManager::GetAsset<Atom::AnimationController>(uuid, true));
        }
    }
}