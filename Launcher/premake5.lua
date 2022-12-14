
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "Launcher"
	location "."
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++latest"
	debugdir "../Resources"

	targetdir ("../bin/" .. outputdir .."/%{prj.name}")
	objdir ("../bin-int/" .. outputdir .."/%{prj.name}")

	disablewarnings
	{
		"4005"
	}

	linkoptions 
	{
		"/ignore:4006",
		"/ignore:4099",
		"/ignore:4098",
	}

    defines
    {
        "GLFW_INCLUDE_NONE",
		"GLM_FORCE_DEPTH_ZERO_TO_ONE",
		"GLM_FORCE_SSE2",
		"NOMINMAX"
    }

	files
	{
		"src/**.h",
		"src/**.cpp",
		"src/**.hpp",
	}

	includedirs
	{
		"src/",
		"../Lamp-Raytracer/src/",

        "%{IncludeDir.VulkanSDK}",
        "%{IncludeDir.GLFW}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.yaml}",
		"%{IncludeDir.stb}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.Optick}",
		"%{IncludeDir.TinyGLTF}",
		"%{IncludeDir.vma}"
	}

    links
    {
        "Lamp-Raytracer",

		"GLFW",
		"ImGui",
		"Optick",

        "%{Library.Vulkan}",
		"%{Library.dxc}"
    }

	filter "system:windows"
		systemversion "latest"

		filter "configurations:Debug"
			defines { "LP_DEBUG" }
			runtime "Debug"
			symbols "on"
			optimize "off"

            links
			{
				"%{Library.ShaderC_Debug}",
				"%{Library.ShaderC_Utils_Debug}",
				"%{Library.SPIRV_Cross_Debug}",
				"%{Library.SPIRV_Cross_GLSL_Debug}",
				"%{Library.SPIRV_Tools_Debug}",

				"%{Library.VulkanUtils}"
			}

		filter "configurations:Release"
			defines { "LP_RELEASE", "NDEBUG" }
			runtime "Release"
			optimize "on"
			symbols "on"

            links
			{
				"%{Library.ShaderC_Release}",
				"%{Library.ShaderC_Utils_Release}",
				"%{Library.SPIRV_Cross_Release}",
				"%{Library.SPIRV_Cross_GLSL_Release}",
			}

		filter "configurations:Dist"
			defines { "LP_DIST", "NDEBUG" }
			runtime "Release"
			optimize "on"
			symbols "off"
			kind "WindowedApp"

            links
			{
				"%{Library.ShaderC_Release}",
				"%{Library.ShaderC_Utils_Release}",
				"%{Library.SPIRV_Cross_Release}",
				"%{Library.SPIRV_Cross_GLSL_Release}",
			}