VULKAN_SDK = os.getenv("VULKAN_SDK")

IncludeDir = {}
IncludeDir["GLFW"] = "%{wks.location}/Lamp-Raytracer/vendor/GLFW/include"
IncludeDir["ImGui"] = "%{wks.location}/Lamp-Raytracer/vendor/ImGui"
IncludeDir["Wire"] = "%{wks.location}/Lamp-Raytracer/vendor/Wire/Wire/src"
IncludeDir["spdlog"] = "%{wks.location}/Lamp-Raytracer/vendor/spdlog/include"
IncludeDir["vma"] = "%{wks.location}/Lamp-Raytracer/vendor/vma"
IncludeDir["VulkanSDK"] = "%{VULKAN_SDK}/Include"
IncludeDir["glm"] = "%{wks.location}/Lamp-Raytracer/vendor/glm"
IncludeDir["yaml"] = "%{wks.location}/Lamp-Raytracer/vendor/yaml-cpp/include"
IncludeDir["fbxsdk"] = "%{wks.location}/Lamp-Raytracer/vendor/fbxsdk/include"
IncludeDir["stb"] = "%{wks.location}/Lamp-Raytracer/vendor/stb"
IncludeDir["shaderc_utils"] = "%{wks.location}/Lamp-Raytracer/vendor/shaderc/libshaderc_util/include"
IncludeDir["shaderc_glslc"] = "%{wks.location}/Lamp-Raytracer/vendor/shaderc/glslc"
IncludeDir["Optick"] = "%{wks.location}/Lamp-Raytracer/vendor/Optick/src"
IncludeDir["TinyGLTF"] = "%{wks.location}/Lamp-Raytracer/vendor/tiny_gltf/"
IncludeDir["tinyddsloader"] = "%{wks.location}/Lamp-Raytracer/vendor/tinyddsloader/"
IncludeDir["imgui_notify"] = "%{wks.location}/Lamp-Raytracer/vendor/imgui-notify/"
IncludeDir["imgui_node_editor"] = "%{wks.location}/Lamp-Raytracer/vendor/imgui-node-editor/"
IncludeDir["ImGuizmo"] = "%{wks.location}/Lamp-Raytracer/vendor/ImGuizmo/"
IncludeDir["P4"] = "%{wks.location}/Sandbox/vendor/p4/include/"

LibraryDir = {}
LibraryDir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"
LibraryDir["VulkanSDK_Debug"] = "%{VULKAN_SDK}/Lib"
LibraryDir["fbxsdk"] = "%{wks.location}/Lamp-Raytracer/vendor/fbxsdk/lib/%{cfg.buildcfg}"
LibraryDir["P4"] = "%{wks.location}/Sandbox/vendor/p4/lib/%{cfg.buildcfg}"
LibraryDir["OpenSSL"] = "%{wks.location}/Sandbox/vendor/OpenSSL/lib/%{cfg.buildcfg}"

Library = {}
Library["Vulkan"] = "%{LibraryDir.VulkanSDK}/vulkan-1.lib"
Library["VulkanUtils"] = "%{LibraryDir.VulkanSDK_Debug}/VkLayer_utils.lib"
Library["dxc"] = "%{LibraryDir.VulkanSDK}/dxcompiler.lib"

Library["ShaderC_Debug"] = "%{LibraryDir.VulkanSDK_Debug}/shaderc_sharedd.lib"
Library["ShaderC_Utils_Debug"] = "%{LibraryDir.VulkanSDK_Debug}/shaderc_utild.lib"
Library["SPIRV_Cross_Debug"] = "%{LibraryDir.VulkanSDK_Debug}/spirv-cross-cored.lib"
Library["SPIRV_Cross_GLSL_Debug"] = "%{LibraryDir.VulkanSDK_Debug}/spirv-cross-glsld.lib"
Library["SPIRV_Tools_Debug"] = "%{LibraryDir.VulkanSDK_Debug}/SPIRV-Toolsd.lib"

Library["ShaderC_Release"] = "%{LibraryDir.VulkanSDK}/shaderc_shared.lib"
Library["ShaderC_Utils_Release"] = "%{LibraryDir.VulkanSDK}/shaderc_util.lib"
Library["SPIRV_Cross_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-core.lib"
Library["SPIRV_Cross_GLSL_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsl.lib"


