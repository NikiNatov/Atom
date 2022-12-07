project "AtomEditor"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"
	characterset("ASCII")

	targetdir("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/**.cpp",
		"src/**.h"
	}

	includedirs
	{
		"%{wks.location}/Atom/src",
		"%{IncludeDirs.spd_log}",
		"%{IncludeDirs.imgui}",
		"%{IncludeDirs.glm}",
		"%{IncludeDirs.PIX}",
		"%{IncludeDirs.entt}",
		"%{IncludeDirs.pybind11}",
		"%{IncludeDirs.python}",
		"%{IncludeDirs.filewatch}",
		"%{IncludeDirs.imguizmo}"
	}

	libdirs
	{
		"%{LibDirs.python}",
	}

	links
	{
		"Atom",
		"%{Libs.python}",
	}

	postbuildcommands
	{
		"XCOPY %{wks.location}\\Atom\\vendor\\PIX\\lib\\WinPixEventRuntime.dll \"%{cfg.targetdir}\"  /S /Y"
	}

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

		defines
		{
			"ATOM_DEBUG",
			"_DEBUG"
		}

		postbuildcommands
		{
			"XCOPY %{wks.location}\\Atom\\vendor\\assimp\\lib\\Debug\\assimp-vc143-mtd.dll \"%{cfg.targetdir}\"  /S /Y"
		}

	filter "configurations:Release"
		defines "ATOM_RELEASE"
		runtime "Release"
		optimize "on"

		defines
		{
			"ATOM_RELEASE",
			"NDEBUG"
		}

		postbuildcommands
		{
			"XCOPY %{wks.location}\\Atom\\vendor\\assimp\\lib\\Release\\assimp-vc143-mt.dll \"%{cfg.targetdir}\"  /S /Y"
		}