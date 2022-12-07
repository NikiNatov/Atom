function queryTerminal(command)
    local success, handle = pcall(io.popen, command)
    if not success then 
        return ""
    end

    result = handle:read("*a")
    handle:close()
    result = string.gsub(result, "\n$", "") -- remove trailing whitespace
    return result
end

function getPythonPath()
    local p = queryTerminal('python -c "import sys; import os; print(os.path.dirname(sys.executable))"')
    
    -- sanitize path before returning it
    p = string.gsub(p, "\\\\", "\\") -- replace double backslash
    p = string.gsub(p, "\\", "/") -- flip slashes
    return p
end

function getPythonLib()
    return queryTerminal("python -c \"import sys; import os; import glob; path = os.path.dirname(sys.executable); libs = glob.glob(path + '/libs/python*'); print(os.path.splitext(os.path.basename(libs[0]))[0]);\"")
end

pythonPath		  = getPythonPath()
pythonIncludePath = pythonPath .. "/include/"
pythonLibPath     = pythonPath .. "/libs/"
pythonLib         = getPythonLib()

if pythonPath == "" or pythonLib == "" then
    error("Failed to find python path!")
else
    print("Python includes: " .. pythonIncludePath)
    print("Python libs: " .. pythonLibPath)
    print("lib: " .. pythonLib)
end

IncludeDirs = {}
IncludeDirs["spd_log"] = "%{wks.location}/Atom/vendor/spdlog/include"
IncludeDirs["glm"] = "%{wks.location}/Atom/vendor/glm"
IncludeDirs["imgui"] = "%{wks.location}/Atom/vendor/imgui"
IncludeDirs["PIX"] = "%{wks.location}/Atom/vendor/PIX/include"
IncludeDirs["stb_image"] = "%{wks.location}/Atom/vendor/stb_image"
IncludeDirs["assimp"] = "%{wks.location}/Atom/vendor/assimp/include"
IncludeDirs["entt"] = "%{wks.location}/Atom/vendor/entt/include"
IncludeDirs["pybind11"] = "%{wks.location}/Atom/vendor/pybind11/include"
IncludeDirs["python"] = pythonIncludePath
IncludeDirs["filewatch"] = "%{wks.location}/Atom/vendor/filewatch"
IncludeDirs["physX"] = "%{wks.location}/Atom/vendor/physX/include"
IncludeDirs["imguizmo"] = "%{wks.location}/Atom/vendor/imguizmo"

LibDirs = {}
LibDirs["PIX"] = "%{wks.location}/Atom/vendor/PIX/lib"
LibDirs["assimp"] = "%{wks.location}/Atom/vendor/assimp/lib"
LibDirs["python"] = pythonLibPath
LibDirs["physX"] = "%{wks.location}/Atom/vendor/physX/lib"

Libs = {}
Libs["PIX"] = "%{LibDirs.PIX}/WinPixEventRuntime.lib"
Libs["assimp_debug"] = "%{LibDirs.assimp}/Debug/assimp-vc143-mtd.lib"
Libs["assimp_release"] = "%{LibDirs.assimp}/Release/assimp-vc143-mt.lib"
Libs["python"] = "%{LibDirs.python}/" .. pythonLib
Libs["physX_debug"] = "%{LibDirs.physX}/Debug/PhysX_static_64.lib"
Libs["physX_character_kinematic_debug"] = "%{LibDirs.physX}/Debug/PhysXCharacterKinematic_static_64.lib"
Libs["physX_common_debug"] = "%{LibDirs.physX}/Debug/PhysXCommon_static_64.lib"
Libs["physX_cooking_debug"] = "%{LibDirs.physX}/Debug/PhysXCooking_static_64.lib"
Libs["physX_extensions_debug"] = "%{LibDirs.physX}/Debug/PhysXExtensions_static_64.lib"
Libs["physX_foundation_debug"] = "%{LibDirs.physX}/Debug/PhysXFoundation_static_64.lib"
Libs["physX_pvd_debug"] = "%{LibDirs.physX}/Debug/PhysXPvdSDK_static_64.lib"
Libs["physX_vehicle_debug"] = "%{LibDirs.physX}/Debug/PhysXVehicle_static_64.lib"
Libs["physX_release"] = "%{LibDirs.physX}/Release/PhysX_static_64.lib"
Libs["physX_character_kinematic_release"] = "%{LibDirs.physX}/Release/PhysXCharacterKinematic_static_64.lib"
Libs["physX_common_release"] = "%{LibDirs.physX}/Release/PhysXCommon_static_64.lib"
Libs["physX_cooking_release"] = "%{LibDirs.physX}/Release/PhysXCooking_static_64.lib"
Libs["physX_extensions_release"] = "%{LibDirs.physX}/Release/PhysXExtensions_static_64.lib"
Libs["physX_foundation_release"] = "%{LibDirs.physX}/Release/PhysXFoundation_static_64.lib"
Libs["physX_pvd_release"] = "%{LibDirs.physX}/Release/PhysXPvdSDK_static_64.lib"
Libs["physX_vehicle_release"] = "%{LibDirs.physX}/Release/PhysXVehicle_static_64.lib"

workspace "Atom"
	architecture "x64"
	startproject "AtomEditor"

	configurations 
	{
		"Debug",
		"Release"
	}

	flags
	{
		"MultiProcessorCompile"
	}

	outputdir = "%{cfg.buildcfg} - %{cfg.system}"

	include "Atom/vendor/imgui"
	include "Atom"
	include "AtomEditor"
	include "Sandbox"