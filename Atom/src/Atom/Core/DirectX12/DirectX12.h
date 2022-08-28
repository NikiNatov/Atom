#pragma once

#include "Atom/Core/Core.h"

#include <dxgi1_6.h>
#include <d3d12.h>
#include <wrl.h>
#include "DXError/dxerr.h"
#include "d3dx12.h"
#include <pix3.h>

#if defined(ATOM_DEBUG)
#include <dxgidebug.h>
#endif

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

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
