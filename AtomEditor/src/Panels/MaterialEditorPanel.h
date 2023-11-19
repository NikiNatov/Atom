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
		void SetMaterial(const Ref<Material>& material);
	private:
		Ref<Material> m_Material;
	};
}