#include "Launcher.h"

#include <Lamp/Core/Base.h>
#include <Lamp/Rendering/Texture/Image2D.h>
#include <Lamp/Utility/UIUtility.h>

#include <imgui.h>

namespace Launcher
{
	void LauncherLayer::OnAttach()
	{
		Lamp::ImageSpecification spec{};
		spec.width = 1024;
		spec.height = 1024;
		spec.format = Lamp::ImageFormat::RGBA;
		spec.usage = Lamp::ImageUsage::Texture;

		m_renderedImage = Lamp::Image2D::Create(spec);

		m_imageBuffer = new uint8_t[1024 * 1024 * 4];
	}

	void LauncherLayer::OnDetach()
	{
		delete[] m_imageBuffer;
		m_renderedImage = nullptr;
	}

	void LauncherLayer::OnEvent(Lamp::Event & e)
	{
		Lamp::EventDispatcher dispatcher(e);
		dispatcher.Dispatch<Lamp::AppImGuiUpdateEvent>(LP_BIND_EVENT_FN(LauncherLayer::OnImGuiUpdate));
		dispatcher.Dispatch<Lamp::AppRenderEvent>(LP_BIND_EVENT_FN(LauncherLayer::OnRender));
	}

	bool LauncherLayer::OnImGuiUpdate(Lamp::AppImGuiUpdateEvent & e)
	{
		ImGui::Begin("TestWindow");

		ImGui::Image(UI::GetTextureID(m_renderedImage), { 512, 512 });

		ImGui::End();

		return false;
	}

	bool LauncherLayer::OnRender(Lamp::AppRenderEvent& e)
	{
		constexpr size_t size = 1024 * 1024 * 4;

		memset(m_imageBuffer, 1, size);
		m_renderedImage->SetData(m_imageBuffer, (uint32_t)size);
		return false;
	}
}