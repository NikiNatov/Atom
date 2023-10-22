project "AtomSIGCompiler"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"
	characterset("ASCII")

	targetdir("%{wks.location}/bin")
	objdir("%{wks.location}/bin-int")

	files
	{
		"src/**.cpp",
		"src/**.h"
	}

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"
		targetname("AtomSIGCompiler_debug")


		defines
		{
			"_DEBUG",
			"DEBUG",
		}

	filter "configurations:Release"
		runtime "Release"
		optimize "on"
		targetname("AtomSIGCompiler")

		defines
		{
			"NDEBUG",
			"RELEASE",
		}