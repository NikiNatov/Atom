#pragma once

#include "Atom/Core/Core.h"

#if defined(ATOM_PLATFORM_WINDOWS)

#include <dxgi1_6.h>
#include <d3d12.h>
#include <wrl.h>
#include "DXError/dxerr.h"
#include "DX12Utils.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

namespace wrl = Microsoft::WRL;

#if defined(ATOM_DEBUG)

	#define DXCall(hr)													\
		if(FAILED(hr))													\
		{																\
			char errDescription[512];									\
			DXGetErrorDescription(hr, errDescription, 512);				\
			ATOM_ENGINE_ERROR("[File]: {0}", __FILE__);					\
			ATOM_ENGINE_ERROR("[Line]: {0}", __LINE__);					\
			ATOM_ENGINE_ERROR("[Error]: {0}", DXGetErrorString(hr));	\
			ATOM_ENGINE_ERROR("[Description]: {0}", errDescription);	\
			ATOM_DEBUG_BREAK();											\
		}

#else
	#define DXCall(hr) hr
#endif // ATOM_DEBUG

#endif // ATOM_PLATFORM_WINDOWS