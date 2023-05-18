#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Asset/MaterialAsset.h"

namespace Atom
{
	class MaterialEditorPanel
	{
	public:
		MaterialEditorPanel() = default;

		void OnImGuiRender();
		void SetMaterial(const Ref<MaterialAsset>& material);
	private:
		Ref<MaterialAsset> m_Material;
	};
}