
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

#include "Lamp/Rendering/Framebuffer.h"
#include "Lamp/Rendering/Shader/ShaderRegistry.h"

#include "Lamp/Scene/Hittable.h"

#include "Lamp/Math/Ray.h"

#include "Lamp/Utility/Math.h"
#include "Lamp/Utility/ImageUtility.h"

namespace Lamp
{
	namespace Utility
	{
		const uint32_t ColorToRGBA(const glm::vec4& color)
		{
			const uint8_t r = static_cast<uint8_t>(color.r * 255.f);
			const uint8_t g = static_cast<uint8_t>(color.g * 255.f);
			const uint8_t b = static_cast<uint8_t>(color.b * 255.f);
			const uint8_t a = static_cast<uint8_t>(color.a * 255.f);

			const uint32_t col = (a << 24) | (b << 16) | (g << 8) | r;
			return col;
		}
	}

	void Renderer::Initialize()
	{
		const uint32_t framesInFlight = Application::Get().GetWindow()->GetSwapchain().GetFramesInFlight();

		s_rendererData = CreateScope<RendererData>();
		s_frameDeletionQueues.resize(framesInFlight);
		s_invalidationQueues.resize(framesInFlight);

		s_rendererData->commandBuffer = CommandBuffer::Create(framesInFlight, false);

		s_rendererData->imageBuffer = new uint32_t[1280 * 720];
		s_rendererData->camera = CreateRef<Camera>(60.f, 16.f / 9.f, 0.1f, 100.f);
		s_rendererData->camera->GenerateRayDirections(1280, 720);

		CreateDescriptorPools();
		CreateSamplers();
	}

	void Renderer::Shutdowm()
	{
		for (auto& descriptorPool : s_rendererData->descriptorPools)
		{
			vkDestroyDescriptorPool(GraphicsContext::GetDevice()->GetHandle(), descriptorPool, nullptr);
		}

		delete[] s_rendererData->imageBuffer;
		s_rendererData = nullptr;

		FlushResources(true);
		SamplerLibrary::Shutdown();
	}

	void Renderer::Begin(Ref<Framebuffer> framebuffer)
	{
		LP_PROFILE_FUNCTION();

		const uint32_t currentFrame = Application::Get().GetWindow()->GetSwapchain().GetCurrentFrame();
		LP_VK_CHECK(vkResetDescriptorPool(GraphicsContext::GetDevice()->GetHandle(), s_rendererData->descriptorPools[currentFrame], 0));

		s_rendererData->currentFramebuffer = framebuffer;
		s_rendererData->commandBuffer->Begin();
		FlushResources();

		LP_PROFILE_GPU_EVENT("Rendering Begin");
		s_rendererData->currentFramebuffer->Bind(s_rendererData->commandBuffer->GetCurrentCommandBuffer());
	}

	void Renderer::End()
	{
		LP_PROFILE_FUNCTION();
		LP_PROFILE_GPU_EVENT("Rendering End");

		s_rendererData->currentFramebuffer->Unbind(s_rendererData->commandBuffer->GetCurrentCommandBuffer());
		s_rendererData->commandBuffer->End();

		s_rendererData->renderCommands.clear();
	}

	void Renderer::Submit(Ref<Hittable> object)
	{
		s_rendererData->renderCommands.emplace_back(object);
	}

	void Renderer::Render()
	{
		LP_PROFILE_FUNCTION();

		const uint32_t width = s_rendererData->currentFramebuffer->GetWidth();
		const uint32_t height = s_rendererData->currentFramebuffer->GetHeight();

		const glm::vec3 origin = { 0.f, 0.f, 0.f };

		for (uint32_t y = 0; y < height; y++)
		{
			for (uint32_t x = 0; x < width; x++)
			{
				const glm::vec3 rayDir = s_rendererData->camera->GetRayDirectionAt(x + y * width);
				const Ray ray = { origin, rayDir };
				
				glm::vec3 color{ 0.f };
				bool hasHit = false;

				for (const auto& obj : s_rendererData->renderCommands)
				{
					RaycastHit hit{};
					if (obj->HitTest(ray, -1000.f, 1000.f, hit))
					{
						color = 0.5f * (hit.normal + 1.f);
						hasHit = true;
					}
				}

				if (!hasHit)
				{
					const float t = 0.5f * (rayDir.y + 1.f);
					color = glm::mix(glm::vec3{ 0.5f, 0.7f, 1.f }, glm::vec3{ 0.f }, t);
				}

				s_rendererData->imageBuffer[x + y * width] = Utility::ColorToRGBA({ color.x, color.y, color.z, 1.f });
			}
		}

		s_rendererData->currentFramebuffer->GetColorAttachment(0)->SetData(s_rendererData->imageBuffer, (uint32_t)width * height * 4);
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

	//float Renderer::HitTestSphere(const glm::vec3& center, const float radius, const Ray& ray)
	//{
	//	const glm::vec3 oc = ray.origin - center;
	//	const float a = glm::length2(ray.direction);
	//	const float halfB = glm::dot(oc, ray.direction);
	//	const float c = glm::length2(oc) - radius * radius;

	//	const float discriminant = halfB * halfB - a * c;
	//	
	//	if (discriminant < 0.f)
	//	{
	//		return -1.f;
	//	}
	//
	//	return (-halfB - std::sqrt(discriminant)) / a;
	//}
}