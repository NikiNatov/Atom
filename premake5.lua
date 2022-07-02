workspace "Atom"
	architecture "x64"
	startproject "Sandbox"

	configurations 
	{
		"Debug",
		"Release"
	}

	outputdir = "%{cfg.buildcfg} - %{cfg.system}"

	IncludeDirs = {}
	IncludeDirs["spd_log"] = "Atom/vendor/spdlog/include"
	IncludeDirs["glm"] = "Atom/vendor/glm"
	IncludeDirs["imgui"] = "Atom/vendor/imgui"

	include "Atom/vendor/imgui"

project "Atom"
	location "Atom"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	systemversion "latest"
	staticruntime "on"
	characterset("ASCII")

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
		"%{prj.name}/src",
		"%{IncludeDirs.spd_log}",
		"%{IncludeDirs.glm}",
		"%{IncludeDirs.imgui}"
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS"
	}

	links
	{
		"ImGui"
	}


	filter "configurations:Debug"
		defines "ATOM_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "ATOM_RELEASE"
		runtime "Release"
		optimize "on"

project "Sandbox"
	location "Sandbox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"
	characterset("ASCII")

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
		"Atom/vendor",
		"%{IncludeDirs.spd_log}",
		"%{IncludeDirs.glm}",
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
