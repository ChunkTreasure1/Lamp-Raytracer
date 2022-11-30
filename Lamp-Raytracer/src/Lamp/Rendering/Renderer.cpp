
#include "lppch.h"
#include "Renderer.h"

#include "Lamp/Core/Application.h"
#include "Lamp/Core/Window.h"
#include "Lamp/Core/Graphics/Swapchain.h"
#include "Lamp/Core/Graphics/GraphicsContext.h"
#include "Lamp/Core/Graphics/GraphicsDevice.h"

#include "Lamp/Log/Log.h"

#include "Lamp/Rendering/Buffer/CommandBuffer.h"
#include "Lamp/Rendering/Camera/Camera.h"

#include "Lamp/Rendering/Texture/SamplerLibrary.h"
#include "Lamp/Rendering/Texture/Texture2D.h"

#include "Lamp/Rendering/Shader/ShaderRegistry.h"

#include "Lamp/Utility/Math.h"
#include "Lamp/Utility/ImageUtility.h"

namespace Lamp
{
	void Renderer::Initialize()
	{
		const uint32_t framesInFlight = Application::Get().GetWindow()->GetSwapchain().GetFramesInFlight();

		s_rendererData = CreateScope<RendererData>();
		s_frameDeletionQueues.resize(framesInFlight);
		s_invalidationQueues.resize(framesInFlight);

		s_rendererData->commandBuffer = CommandBuffer::Create(framesInFlight, false);

		CreateDescriptorPools();
		CreateSamplers();
	}

	void Renderer::Shutdowm()
	{
		for (auto& descriptorPool : s_rendererData->descriptorPools)
		{
			vkDestroyDescriptorPool(GraphicsContext::GetDevice()->GetHandle(), descriptorPool, nullptr);
		}

		s_rendererData = nullptr;

		FlushResources(true);

		SamplerLibrary::Shutdown();
	}

	void Renderer::Begin()
	{
		LP_PROFILE_FUNCTION();

		const uint32_t currentFrame = Application::Get().GetWindow()->GetSwapchain().GetCurrentFrame();
		LP_VK_CHECK(vkResetDescriptorPool(GraphicsContext::GetDevice()->GetHandle(), s_rendererData->descriptorPools[currentFrame], 0));

		s_rendererData->commandBuffer->Begin();

		LP_PROFILE_GPU_EVENT("Rendering Begin");

		FlushResources();
	}

	void Renderer::End()
	{
		LP_PROFILE_FUNCTION();
		LP_PROFILE_GPU_EVENT("Rendering Begin");
		s_rendererData->commandBuffer->End();
	}

	void Renderer::FlushResources(bool flushAll)
	{
		if (!flushAll) [[likely]]
		{
			const uint32_t currentFrame = Application::Get().GetWindow()->GetSwapchain().GetCurrentFrame();
			s_frameDeletionQueues[currentFrame].Flush();
			s_invalidationQueues[currentFrame].Flush();
		}
		else
		{
			const uint32_t framesInFlight = Application::Get().GetWindow()->GetSwapchain().GetFramesInFlight();

			for (uint32_t i = 0; i < framesInFlight; i++)
			{
				s_invalidationQueues[i].Flush();
				s_frameDeletionQueues[i].Flush();
			}
		}
	}

	void Renderer::SubmitResourceFree(std::function<void()>&& function)
	{
		const uint32_t currentFrame = Application::Get().GetWindow()->GetSwapchain().GetCurrentFrame();
		s_frameDeletionQueues[currentFrame].Push(function);
	}

	void Renderer::SubmitInvalidation(std::function<void()>&& function)
	{
		const uint32_t currentFrame = Application::Get().GetWindow()->GetSwapchain().GetCurrentFrame();
		s_invalidationQueues[currentFrame].Push(function);
	}

	VkDescriptorSet Renderer::AllocateDescriptorSet(VkDescriptorSetAllocateInfo& allocInfo)
	{
		LP_PROFILE_FUNCTION();

		auto device = GraphicsContext::GetDevice();
		uint32_t currentFrame = Application::Get().GetWindow()->GetSwapchain().GetCurrentFrame();

		allocInfo.descriptorPool = s_rendererData->descriptorPools[currentFrame];

		VkDescriptorSet descriptorSet;
		LP_VK_CHECK(vkAllocateDescriptorSets(device->GetHandle(), &allocInfo, &descriptorSet));

		return descriptorSet;
	}

	void Renderer::CreateSamplers()
	{
		SamplerLibrary::Add(TextureFilter::Linear, TextureFilter::Linear, TextureFilter::Linear, TextureWrap::Repeat, CompareOperator::None, AniostopyLevel::None);
		SamplerLibrary::Add(TextureFilter::Nearest, TextureFilter::Linear, TextureFilter::Linear, TextureWrap::Repeat, CompareOperator::None, AniostopyLevel::None);
		SamplerLibrary::Add(TextureFilter::Nearest, TextureFilter::Nearest, TextureFilter::Linear, TextureWrap::Repeat, CompareOperator::None, AniostopyLevel::None);

		SamplerLibrary::Add(TextureFilter::Nearest, TextureFilter::Nearest, TextureFilter::Nearest, TextureWrap::Repeat, CompareOperator::None, AniostopyLevel::None);
		SamplerLibrary::Add(TextureFilter::Linear, TextureFilter::Nearest, TextureFilter::Nearest, TextureWrap::Repeat, CompareOperator::None, AniostopyLevel::None);
		SamplerLibrary::Add(TextureFilter::Linear, TextureFilter::Linear, TextureFilter::Nearest, TextureWrap::Repeat, CompareOperator::None, AniostopyLevel::None);

		SamplerLibrary::Add(TextureFilter::Linear, TextureFilter::Linear, TextureFilter::Linear, TextureWrap::Clamp, CompareOperator::None, AniostopyLevel::None);
		SamplerLibrary::Add(TextureFilter::Nearest, TextureFilter::Linear, TextureFilter::Linear, TextureWrap::Clamp, CompareOperator::None, AniostopyLevel::None);
		SamplerLibrary::Add(TextureFilter::Nearest, TextureFilter::Nearest, TextureFilter::Linear, TextureWrap::Clamp, CompareOperator::None, AniostopyLevel::None);

		SamplerLibrary::Add(TextureFilter::Nearest, TextureFilter::Nearest, TextureFilter::Nearest, TextureWrap::Clamp, CompareOperator::None, AniostopyLevel::None);
		SamplerLibrary::Add(TextureFilter::Linear, TextureFilter::Nearest, TextureFilter::Nearest, TextureWrap::Clamp, CompareOperator::None, AniostopyLevel::None);
		SamplerLibrary::Add(TextureFilter::Linear, TextureFilter::Linear, TextureFilter::Nearest, TextureWrap::Clamp, CompareOperator::None, AniostopyLevel::None);
	}

	void Renderer::CreateDescriptorPools()
	{
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

		const uint32_t framesInFlight = Application::Get().GetWindow()->GetSwapchain().GetFramesInFlight();

		s_rendererData->descriptorPools.resize(framesInFlight);
		auto device = GraphicsContext::GetDevice();

		for (auto& descriptorPool : s_rendererData->descriptorPools)
		{
			LP_VK_CHECK(vkCreateDescriptorPool(device->GetHandle(), &poolInfo, nullptr, &descriptorPool));
		}
	}
}