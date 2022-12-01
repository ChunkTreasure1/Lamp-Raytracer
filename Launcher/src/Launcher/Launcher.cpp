#include "Launcher.h"

#include "Launcher/Objects/Sphere.h"

#include <Lamp/Core/Base.h>
#include <Lamp/Utility/UIUtility.h>

#include <Lamp/Rendering/Texture/Image2D.h>
#include <Lamp/Rendering/Renderer.h>
#include <Lamp/Rendering/Framebuffer.h>

#include <Lamp/Scene/Scene.h>

#include <imgui.h>

namespace Launcher
{
	void LauncherLayer::OnAttach()
	{
		Lamp::FramebufferSpecification spec{};
		spec.attachments =
		{
			{ Lamp::ImageFormat::RGBA }
		};

		spec.width = 1280;
		spec.height = 720;
	
		m_framebuffer = Lamp::Framebuffer::Create(spec);

		m_scene = CreateRef<Lamp::Scene>();

		m_scene->AddObject(CreateRef<Sphere>(glm::vec3{ 2.f, 0.f, -5.f }, 0.5f));
		m_scene->AddObject(CreateRef<Sphere>(glm::vec3{ -2.f, 0.f, -5.f }, 0.5f));
		//m_scene->AddObject(CreateRef<Sphere>(glm::vec3{ 0.f, -100.f, -1.f }, 100.f));
	}

	void LauncherLayer::OnDetach()
	{
		m_framebuffer = nullptr;
	}

	void LauncherLayer::OnEvent(Lamp::Event& e)
	{
		Lamp::EventDispatcher dispatcher(e);
		dispatcher.Dispatch<Lamp::AppImGuiUpdateEvent>(LP_BIND_EVENT_FN(LauncherLayer::OnImGuiUpdate));
		dispatcher.Dispatch<Lamp::AppRenderEvent>(LP_BIND_EVENT_FN(LauncherLayer::OnRender));
	}

	bool LauncherLayer::OnImGuiUpdate(Lamp::AppImGuiUpdateEvent& e)
	{
		ImGui::Begin("TestWindow");

		ImGui::Image(UI::GetTextureID(m_framebuffer->GetColorAttachment(0)), { 1280, 720 });

		ImGui::End();

		return false;
	}

	bool LauncherLayer::OnRender(Lamp::AppRenderEvent& e)
	{
		m_scene->OnRender();

		Lamp::Renderer::Begin(m_framebuffer);
		Lamp::Renderer::Render();
		Lamp::Renderer::End();

		return false;
	}
}