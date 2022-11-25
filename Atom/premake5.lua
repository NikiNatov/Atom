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
	}

	includedirs
	{
		"src",
		"%{IncludeDirs.spd_log}",
		"%{IncludeDirs.glm}",
		"%{IncludeDirs.imgui}",
		"%{IncludeDirs.PIX}",
		"%{IncludeDirs.stb_image}",
		"%{IncludeDirs.assimp}",
		"%{IncludeDirs.entt}",
		"%{IncludeDirs.pybind11}",
		"%{IncludeDirs.python}",
	}

	libdirs
	{
		"%{LibDirs.PIX}",
		"%{LibDirs.assimp}",
		"%{LibDirs.python}",
	}

	links
	{
		"ImGui",
		"%{Libs.PIX}",
		"%{Libs.python}",
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS"
	}

	filter "configurations:Debug"
		defines "ATOM_DEBUG"
		runtime "Debug"
		symbols "on"

		links
		{
			"%{Libs.assimp_debug}" 
		}

	filter "configurations:Release"
		defines "ATOM_RELEASE"
		runtime "Release"
		optimize "on"

		links
		{
			"%{Libs.assimp_release}"  
		}