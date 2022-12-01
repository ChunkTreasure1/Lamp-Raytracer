#pragma once

#include "Lamp/Core/Base.h"

#include "Lamp/Rendering/FunctionQueue.hpp"

#include <vulkan/vulkan.h>
#include <functional>

namespace Lamp
{
	class Camera;
	class CommandBuffer;
	class Framebuffer;
	class Hittable;

	class Renderer
	{
	public:
		static void Initialize();
		static void Shutdowm();

		static void Begin(Ref<Framebuffer> framebuffer);
		static void End();

		static void Submit(Ref<Hittable> object);
		static void Render();

		static void FlushResources(bool flushAll = false);

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
			Ref<Framebuffer> currentFramebuffer;

			Ref<Camera> camera;
			std::vector<VkDescriptorPool> descriptorPools;

			uint32_t* imageBuffer = nullptr;
			std::vector<Ref<Hittable>> renderCommands;
		};

		inline static Scope<RendererData> s_rendererData;
		inline static std::vector<FunctionQueue> s_frameDeletionQueues;
		inline static std::vector<FunctionQueue> s_invalidationQueues;
	};
}