project "Sandbox"
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
		"%{wks.location}/Atom/vendor",
		"%{IncludeDirs.spd_log}",
		"%{IncludeDirs.glm}",
		"%{IncludeDirs.PIX}",
		"%{IncludeDirs.entt}",
		"%{IncludeDirs.pybind11}",
		"%{IncludeDirs.python}",
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
		"XCOPY %{wks.location}\\Atom\\vendor\\PIX\\lib\\WinPixEventRuntime.dll \"%{cfg.targetdir}\" /S /Y"
	}

	filter "configurations:Debug"
		defines "ATOM_DEBUG"
		runtime "Debug"
		symbols "on"

		postbuildcommands
		{
			"XCOPY %{wks.location}\\Atom\\vendor\\assimp\\lib\\Debug\\assimp-vc143-mtd.dll \"%{cfg.targetdir}\"  /S /Y"
		}

	filter "configurations:Release"
		defines "ATOM_RELEASE"
		runtime "Release"
		optimize "on"

		postbuildcommands
		{
			"XCOPY %{wks.location}\\Atom\\vendor\\assimp\\lib\\Release\\assimp-vc143-mt.dll \"%{cfg.targetdir}\"  /S /Y"
		}