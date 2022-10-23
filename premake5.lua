workspace "Atom"
	architecture "x64"
	startproject "AtomEditor"

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
	IncludeDirs["PIX"] = "Atom/vendor/PIX/include"
	IncludeDirs["stb_image"] = "Atom/vendor/stb_image"
	IncludeDirs["assimp"] = "Atom/vendor/assimp/include"
	IncludeDirs["entt"] = "Atom/vendor/entt/include"

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
		"%{prj.name}/src/**.h",
		"%{prj.name}/vendor/stb_image/**.h",
		"%{prj.name}/vendor/stb_image/**.cpp",
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{IncludeDirs.spd_log}",
		"%{IncludeDirs.glm}",
		"%{IncludeDirs.imgui}",
		"%{IncludeDirs.PIX}",
		"%{IncludeDirs.stb_image}",
		"%{IncludeDirs.assimp}",
		"%{IncludeDirs.entt}",
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS"
	}

	links
	{
		"ImGui",
		"Atom/vendor/PIX/lib/WinPixEventRuntime.lib"
	}


	filter "configurations:Debug"
		defines "ATOM_DEBUG"
		runtime "Debug"
		symbols "on"

		links
		{
			"Atom/vendor/assimp/lib/Debug/assimp-vc143-mtd.lib" 
		}

	filter "configurations:Release"
		defines "ATOM_RELEASE"
		runtime "Release"
		optimize "on"

		links
		{
			"Atom/vendor/assimp/lib/Release/assimp-vc143-mt.lib" 
		}

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
		"%{IncludeDirs.PIX}",
		"%{IncludeDirs.entt}",
	}

	links
	{
		"Atom"
	}

	postbuildcommands
	{
		"XCOPY ..\\Atom\\vendor\\PIX\\lib\\WinPixEventRuntime.dll \"%{cfg.targetdir}\"  /S /Y"
	}

	filter "configurations:Debug"
		defines "ATOM_DEBUG"
		runtime "Debug"
		symbols "on"

		postbuildcommands
		{
			"XCOPY ..\\Atom\\vendor\\assimp\\lib\\Debug\\assimp-vc143-mtd.dll \"%{cfg.targetdir}\"  /S /Y"
		}

	filter "configurations:Release"
		defines "ATOM_RELEASE"
		runtime "Release"
		optimize "on"

		postbuildcommands
		{
			"XCOPY ..\\Atom\\vendor\\assimp\\lib\\Release\\assimp-vc143-mt.dll \"%{cfg.targetdir}\"  /S /Y"
		}

project "AtomEditor"
	location "AtomEditor"
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
		"%{IncludeDirs.PIX}",
		"%{IncludeDirs.entt}",
	}

	links
	{
		"Atom"
	}

	postbuildcommands
	{
		"XCOPY ..\\Atom\\vendor\\PIX\\lib\\WinPixEventRuntime.dll \"%{cfg.targetdir}\"  /S /Y"
	}

	filter "configurations:Debug"
		defines "ATOM_DEBUG"
		runtime "Debug"
		symbols "on"

		postbuildcommands
		{
			"XCOPY ..\\Atom\\vendor\\assimp\\lib\\Debug\\assimp-vc143-mtd.dll \"%{cfg.targetdir}\"  /S /Y"
		}

	filter "configurations:Release"
		defines "ATOM_RELEASE"
		runtime "Release"
		optimize "on"

		postbuildcommands
		{
			"XCOPY ..\\Atom\\vendor\\assimp\\lib\\Release\\assimp-vc143-mt.dll \"%{cfg.targetdir}\"  /S /Y"
		}