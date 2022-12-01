#pragma once

#include <Lamp/Event/ApplicationEvent.h>
#include <Lamp/Core/Layer/Layer.h>

namespace Lamp
{
	class Scene;
	class Framebuffer;
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

		Ref<Lamp::Framebuffer> m_framebuffer;
		Ref<Lamp::Scene> m_scene;
	};
}