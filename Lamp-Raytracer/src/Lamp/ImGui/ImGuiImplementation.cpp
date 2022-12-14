#include "lppch.h"
#include "ImGuiImplementation.h"

#include "Lamp/Core/Graphics/GraphicsDevice.h"
#include "Lamp/Core/Graphics/GraphicsContext.h"
#include "Lamp/Core/Graphics/Swapchain.h"
#include "Lamp/Core/Application.h"
#include "Lamp/Core/Window.h"

#include "Lamp/Log/Log.h"
#include "Lamp/Utility/FileSystem.h"

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include <imgui.h>

namespace Lamp
{
	static std::vector<VkCommandBuffer> s_imGuiCommandBuffer;

	namespace Utils
	{
		inline void VulkanCheckResult(VkResult result)
		{
			if (result != VK_SUCCESS)
			{
				LP_CORE_ERROR("VkResult is '{0}' in {1}:{2}", VKResultToString(result), __FILE__, __LINE__);
				if (result == VK_ERROR_DEVICE_LOST)
				{
					using namespace std::chrono_literals;
					std::this_thread::sleep_for(3s);
				}
				LP_CORE_ASSERT(result == VK_SUCCESS, "");
			}
		}
	}

	ImGuiImplementation::ImGuiImplementation()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;	//Enable keyboard controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;		//Enable docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;		//Enable multiple viewports
		
		io.ConfigWindowsMoveFromTitleBarOnly = true;
		io.IniFilename = nullptr;

		const std::filesystem::path iniPath = GetOrCreateIniPath();
		ImGui::LoadIniSettingsFromDisk(iniPath.string().c_str());

		m_font = io.Fonts->AddFontFromFileTTF("Engine/Fonts/Futura/futura-light.ttf", 18.f);
		ImGui::StyleColorsDark();

		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.f;
		}

		style.Colors[ImGuiCol_Text] = ImVec4(1.000f, 1.000f, 1.000f, 1.000f);
		style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.500f, 0.500f, 0.500f, 1.000f);

		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.258f, 0.258f, 0.258f, 1.000f);
		style.Colors[ImGuiCol_ChildBg] = ImVec4(0.280f, 0.280f, 0.280f, 0.000f);
		style.Colors[ImGuiCol_PopupBg] = ImVec4(0.313f, 0.313f, 0.313f, 1.000f);

		style.Colors[ImGuiCol_Border] = ImVec4(0.137f, 0.137f, 0.137f, 1.000f);
		style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.000f, 0.000f, 0.000f, 0.000f);

		style.Colors[ImGuiCol_FrameBg] = ImVec4(0.160f, 0.160f, 0.160f, 1.000f);
		style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.200f, 0.200f, 0.200f, 1.000f);
		style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.280f, 0.280f, 0.280f, 1.000f);

		style.Colors[ImGuiCol_TitleBg] = ImVec4(0.137f, 0.137f, 0.137f, 1.000f);
		style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.137f, 0.137f, 0.137f, 1.000f);
		style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.148f, 0.148f, 0.148f, 1.000f);

		style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.195f, 0.195f, 0.195f, 1.000f);

		style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.160f, 0.160f, 0.160f, 1.000f);
		style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.277f, 0.277f, 0.277f, 1.000f);
		style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.300f, 0.300f, 0.300f, 1.000f);
		style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.4f, 0.67f, 1.000f, 1.000f);

		style.Colors[ImGuiCol_CheckMark] = ImVec4(1.000f, 1.000f, 1.000f, 1.000f);

		style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.391f, 0.391f, 0.391f, 1.000f);
		style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.4f, 0.67f, 1.000f, 1.000f);

		style.Colors[ImGuiCol_Button] = ImVec4(0.258f, 0.258f, 0.258f, 1.000f);
		style.Colors[ImGuiCol_ButtonHovered] = ImVec4(1.000f, 1.000f, 1.000f, 0.156f);
		style.Colors[ImGuiCol_ButtonActive] = ImVec4(1.000f, 1.000f, 1.000f, 0.391f);


		style.Colors[ImGuiCol_Header] = ImVec4(0.313f, 0.313f, 0.313f, 1.000f);
		style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.469f, 0.469f, 0.469f, 1.000f);
		style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.469f, 0.469f, 0.469f, 1.000f);

		style.Colors[ImGuiCol_Separator] = style.Colors[ImGuiCol_Border];
		style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.391f, 0.391f, 0.391f, 1.000f);
		style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.4f, 0.67f, 1.000f, 1.000f);

		style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.000f, 1.000f, 1.000f, 0.250f);
		style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.000f, 1.000f, 1.000f, 0.670f);
		style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.4f, 0.67f, 1.000f, 1.000f);

		style.Colors[ImGuiCol_Tab] = ImVec4(0.137f, 0.137f, 0.137f, 1.000f);
		style.Colors[ImGuiCol_TabTop] = ImVec4(0.4f, 0.67f, 1.000f, 1.000f);
		style.Colors[ImGuiCol_TabHovered] = ImVec4(0.352f, 0.352f, 0.352f, 1.000f);
		style.Colors[ImGuiCol_TabActive] = ImVec4(0.258f, 0.258f, 0.258f, 1.000f);
		style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.137f, 0.137f, 0.137f, 1.000f);
		style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.258f, 0.258f, 0.258f, 1.000f);

		style.Colors[ImGuiCol_DockingPreview] = ImVec4(0.4f, 0.67f, 1.000f, 0.781f);
		style.Colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.137f, 0.137f, 0.137f, 1.000f);

		style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.160f, 0.160f, 0.160f, 1.000f);
		style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.160f, 0.160f, 0.160f, 1.000f);
		style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.160f, 0.160f, 0.160f, 1.000f);

		style.Colors[ImGuiCol_PlotLines] = ImVec4(0.469f, 0.469f, 0.469f, 1.000f);
		style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.4f, 0.67f, 1.000f, 1.000f);
		style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.586f, 0.586f, 0.586f, 1.000f);
		style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);

		style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(1.000f, 1.000f, 1.000f, 0.156f);
		style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);

		style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.4f, 0.67f, 1.000f, 1.000f);
		style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.4f, 0.67f, 1.000f, 1.000f);
		style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.000f, 0.000f, 0.000f, 0.586f);
		style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.000f, 0.000f, 0.000f, 0.586f);

		style.ChildRounding = 0;
		style.FrameRounding = 0;
		style.GrabMinSize = 7.0f;
		style.PopupRounding = 2.0f;
		style.ScrollbarRounding = 12.0f;
		style.ScrollbarSize = 13.0f;
		style.TabBorderSize = 0.0f;
		style.TabRounding = 0.0f;
		style.WindowRounding = 0.0f;
		style.WindowBorderSize = 2.f;

		/////Vulkan initialization/////
		auto& context = GraphicsContext::Get();
		auto& swapchain = Application::Get().GetWindow()->GetSwapchain();
		auto device = GraphicsContext::GetDevice();

		VkDescriptorPoolSize poolSizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		poolInfo.maxSets = 10000;
		poolInfo.poolSizeCount = (uint32_t)ARRAYSIZE(poolSizes);
		poolInfo.pPoolSizes = poolSizes;

		LP_VK_CHECK(vkCreateDescriptorPool(device->GetHandle(), &poolInfo, nullptr, &m_descriptorPool));

		Application& app = Application::Get();
		GLFWwindow* pWindow = static_cast<GLFWwindow*>(app.GetWindow()->GetNativeWindow());
		ImGui_ImplGlfw_InitForVulkan(pWindow, true);

		ImGui_ImplVulkan_InitInfo initInfo{};
		initInfo.Instance = static_cast<VkInstance>(context.GetInstance());

		initInfo.PhysicalDevice = static_cast<VkPhysicalDevice>(device->GetPhysicalDevice()->GetHandle());
		initInfo.Device = static_cast<VkDevice>(device->GetHandle());
		initInfo.Queue = device->GetGraphicsQueue();
		initInfo.DescriptorPool = m_descriptorPool;
		initInfo.MinImageCount = 3;
		initInfo.ImageCount = 3;
		initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		initInfo.CheckVkResultFn = Utils::VulkanCheckResult;
		
		ImGui_ImplVulkan_Init(&initInfo, swapchain.GetRenderPass());
		/////////////////////////////////

		VkCommandBuffer commandBuffer = device->GetCommandBuffer(true);
		ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
		device->FlushCommandBuffer(commandBuffer);

		vkDeviceWaitIdle(device->GetHandle());
		ImGui_ImplVulkan_DestroyFontUploadObjects();

		uint32_t framesInFlight = swapchain.GetFramesInFlight();
		s_imGuiCommandBuffer.resize(framesInFlight);

		for (uint32_t i = 0; i < framesInFlight; i++)
		{
			s_imGuiCommandBuffer[i] = device->CreateSecondaryCommandBuffer();
		}
	}

	ImGuiImplementation::~ImGuiImplementation()
	{
		vkDeviceWaitIdle(GraphicsContext::GetDevice()->GetHandle());
		vkDestroyDescriptorPool(GraphicsContext::GetDevice()->GetHandle(), m_descriptorPool, nullptr);
		ImGui_ImplVulkan_Shutdown();
	}

	void ImGuiImplementation::Begin()
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void ImGuiImplementation::End()
	{
		ImGui::Render();


		auto& swapchain = Application::Get().GetWindow()->GetSwapchain();

		VkCommandBuffer drawCmdBuffer;
		VkCommandBuffer secondaryCmdBuffer;

		uint32_t width = Application::Get().GetWindow()->GetWidth();
		uint32_t height = Application::Get().GetWindow()->GetHeight();

		// Begin command buffer
		{
			uint32_t frameIndex = swapchain.GetCurrentFrame();
			
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.pNext = nullptr;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			drawCmdBuffer = swapchain.GetCommandBuffer(frameIndex);
			secondaryCmdBuffer = s_imGuiCommandBuffer[frameIndex];

			LP_VK_CHECK(vkBeginCommandBuffer(drawCmdBuffer, &beginInfo));
		}

		// Begin render pass
		{
			VkClearValue clearValues[2];
			clearValues[0].color = { { 0.1f, 0.1f, 0.1f, 1.f } };
			clearValues[1].depthStencil = { 1.0f, 0 };

			VkRenderPassBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			beginInfo.pNext = nullptr;
			beginInfo.renderPass = swapchain.GetRenderPass();
			beginInfo.renderArea.offset.x = 0;
			beginInfo.renderArea.offset.y = 0;
			beginInfo.renderArea.extent.width = width;
			beginInfo.renderArea.extent.height = height;
			beginInfo.clearValueCount = 2;
			beginInfo.pClearValues = clearValues;
			beginInfo.framebuffer = swapchain.GetCurrentFramebuffer();
		
			vkCmdBeginRenderPass(drawCmdBuffer, &beginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
		}

		// Begin secondary command buffer
		{
			VkCommandBufferInheritanceInfo inheritInfo{};
			inheritInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
			inheritInfo.renderPass = swapchain.GetRenderPass();
			inheritInfo.framebuffer = swapchain.GetCurrentFramebuffer();
		
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
			beginInfo.pInheritanceInfo = &inheritInfo;

			LP_VK_CHECK(vkBeginCommandBuffer(secondaryCmdBuffer, &beginInfo));
		}

		// Viewport
		{
			VkViewport viewport{};
			viewport.x = 0.f;
			viewport.y = (float)height;
			viewport.width = (float)width;
			viewport.height = -(float)height;
			viewport.minDepth = 0.f;
			viewport.maxDepth = 1.f;

			vkCmdSetViewport(secondaryCmdBuffer, 0, 1, &viewport);
		}

		// Scissor
		{
			VkRect2D scissor{};
			scissor.extent.width = width;
			scissor.extent.height = height;
			scissor.offset.x = 0;
			scissor.offset.y = 0;

			vkCmdSetScissor(secondaryCmdBuffer, 0, 1, &scissor);
		}

		ImDrawData* drawData = ImGui::GetDrawData();
		ImGui_ImplVulkan_RenderDrawData(drawData, secondaryCmdBuffer);

		LP_VK_CHECK(vkEndCommandBuffer(secondaryCmdBuffer));

		vkCmdExecuteCommands(drawCmdBuffer, 1, &secondaryCmdBuffer);
		vkCmdEndRenderPass(drawCmdBuffer);

		LP_VK_CHECK(vkEndCommandBuffer(drawCmdBuffer));

		ImGuiIO& io = ImGui::GetIO(); (void)io;

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}

	Scope<ImGuiImplementation> ImGuiImplementation::Create()
	{
		return CreateScope<ImGuiImplementation>();
	}

	std::filesystem::path ImGuiImplementation::GetOrCreateIniPath()
	{
		const std::filesystem::path userIniFile = "User/imgui.ini";
		const std::filesystem::path defaultIniPath = "Editor/imgui.ini";

		if (!FileSystem::Exists(userIniFile))
		{
			LP_CORE_INFO("User Ini file not found! Copying default!");
			
			std::filesystem::create_directories(userIniFile.parent_path());
			if (!FileSystem::Exists(defaultIniPath))
			{
				LP_CORE_ERROR("Unable to find default ini file!");
				
				if (!FileSystem::Exists(defaultIniPath.parent_path()))
				{
					std::filesystem::create_directories(defaultIniPath.parent_path());
				}

				return defaultIniPath;
			}

			FileSystem::Copy(defaultIniPath, userIniFile);
		}

		return userIniFile;
	}
}