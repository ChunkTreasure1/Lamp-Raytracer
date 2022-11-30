#pragma once

#include <Lamp/Event/ApplicationEvent.h>
#include <Lamp/Core/Layer/Layer.h>

namespace Lamp
{
	class Image2D;
}

namespace Launcher
{
	class LauncherLayer : public Lamp::Layer
	{
	public:
		void OnAttach() override;
		void OnDetach() override;

		void OnEvent(Lamp::Event& e) override;

	private:
		bool OnImGuiUpdate(Lamp::AppImGuiUpdateEvent& e);
		bool OnRender(Lamp::AppRenderEvent& e);

		Ref<Lamp::Image2D> m_renderedImage;
		uint8_t* m_imageBuffer = nullptr;
	};
}