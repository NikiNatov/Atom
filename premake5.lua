workspace "Atom"
	architecture "x64"
	startproject "Sandbox"

	configurations 
	{
		"Debug",
		"Release"
	}

	outputdir = "%{cfg.buildcfg} - %{cfg.system}"

project "Atom"
	location "Atom"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	systemversion "latest"
	staticruntime "on"

	targetdir("bin/" .. outputdir .. "/%{prj.name}")
	objdir("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "atompch.h"
	pchsource "%{prj.name}/src/atompch.cpp"

	files
	{
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/src/**.h"
	}

	includedirs
	{
		"%{prj.name}/src"
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS"
	}

	links
	{
	}


	filter "configurations:Debug"
		defines "ATOM_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "ATOM_RELEASE"
		runtime "Release"
		optimize "on"

	filter "system:Windows"
		defines "ATOM_PLATFORM_WINDOWS"

project "Sandbox"
	location "Sandbox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir("bin/" .. outputdir .. "/%{prj.name}")
	objdir("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/src/**.h"
	}

	includedirs
	{
		"Atom/src",
		"Atom/vendor"
	}

	links
	{
		"Atom"
	}

	filter "configurations:Debug"
		defines "ATOM_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "ATOM_RELEASE"
		runtime "Release"
		optimize "on"

	filter "system:Windows"
		defines "ATOM_PLATFORM_WINDOWS"
