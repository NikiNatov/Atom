project "Atom"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	systemversion "latest"
	staticruntime "on"
	characterset("ASCII")

	targetdir("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "atompch.h"
	pchsource "src/atompch.cpp"

	files
	{
		"src/**.cpp",
		"src/**.h",
		"vendor/stb_image/**.h",
		"vendor/stb_image/**.cpp",
		"vendor/imguizmo/**.h",
		"vendor/imguizmo/**.cpp",
	}

	includedirs
	{
		"src",
		"shaders",
		"%{IncludeDirs.spd_log}",
		"%{IncludeDirs.glm}",
		"%{IncludeDirs.imgui}",
		"%{IncludeDirs.PIX}",
		"%{IncludeDirs.stb_image}",
		"%{IncludeDirs.assimp}",
		"%{IncludeDirs.entt}",
		"%{IncludeDirs.pybind11}",
		"%{IncludeDirs.python}",
		"%{IncludeDirs.filewatch}",
		"%{IncludeDirs.physX}",
		"%{IncludeDirs.imguizmo}",
		"%{IncludeDirs.yaml}"
	}

	libdirs
	{
		"%{LibDirs.PIX}",
		"%{LibDirs.assimp}",
		"%{LibDirs.python}",
		"%{LibDirs.physX}",
	}

	links
	{
		"ImGui",
		"Yaml",
		"%{Libs.PIX}",
		"%{Libs.python}",
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"PX_PHYSX_STATIC_LIB",
	}

	buildoptions
	{
		"/bigobj"
	}

	prebuildcommands
	{
		"%{wks.location}/tools/AtomSIGCompiler/bin/AtomSIGCompiler.exe -sigInputDir %{prj.location}/shaders/sig -sigOutputDir %{prj.location}/shaders/autogen"
	}

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

		defines
		{
			"ATOM_DEBUG",
			"_DEBUG"
		}

		links
		{
			"%{Libs.assimp_debug}",
			"%{Libs.physX_debug}", 
			"%{Libs.physX_character_kinematic_debug}", 
			"%{Libs.physX_common_debug}", 
			"%{Libs.physX_cooking_debug}", 
			"%{Libs.physX_extensions_debug}", 
			"%{Libs.physX_foundation_debug}", 
			"%{Libs.physX_pvd_debug}", 
			"%{Libs.physX_vehicle_debug}", 
		}

	filter "configurations:Release"
		runtime "Release"
		optimize "on"

		defines
		{
			"ATOM_RELEASE",
			"NDEBUG"
		}

		links
		{
			"%{Libs.assimp_release}", 
			"%{Libs.physX_release}", 
			"%{Libs.physX_character_kinematic_release}", 
			"%{Libs.physX_common_release}", 
			"%{Libs.physX_cooking_release}", 
			"%{Libs.physX_extensions_release}", 
			"%{Libs.physX_foundation_release}", 
			"%{Libs.physX_pvd_release}", 
			"%{Libs.physX_vehicle_release}", 
		}