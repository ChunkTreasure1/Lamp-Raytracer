#pragma once

#include "Lamp/Core/Base.h"

#include "Lamp/Rendering/FunctionQueue.hpp"

#include <vulkan/vulkan.h>
#include <functional>

namespace Lamp
{
	class Camera;
	class CommandBuffer;

	class Renderer
	{
	public:
		static void Initialize();
		static void Shutdowm();

		static void Begin();
		static void End();

		static void SubmitResourceFree(std::function<void()>&& function);
		static void SubmitInvalidation(std::function<void()>&& function);

		static VkDescriptorSet AllocateDescriptorSet(VkDescriptorSetAllocateInfo& allocInfo);

	private:
		Renderer() = delete;
		
		static void CreateSamplers();

		static void CreateDescriptorPools();

		struct RendererData
		{
			Ref<CommandBuffer> commandBuffer;
			Ref<Camera> passCamera;
			std::vector<VkDescriptorPool> descriptorPools;
		};

		inline static Scope<RendererData> s_rendererData;
		inline static std::vector<FunctionQueue> s_frameDeletionQueues;
		inline static std::vector<FunctionQueue> s_invalidationQueues;
	};
}