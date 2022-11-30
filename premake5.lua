workspace "Lamp-Raytracer"
	architecture "x64"
	startproject "Launcher"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}
	
	flags
	{
		"MultiProcessorCompile"
	}
	
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

include "Dependencies.lua"

group "Dependencies"
include "Lamp-Raytracer/vendor/glfw"
include "Lamp-Raytracer/vendor/imgui"
include "Lamp-Raytracer/vendor/Optick"

group "Core"
include "Lamp-Raytracer"

group ""
include "Launcher"