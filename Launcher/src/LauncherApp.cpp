#include "Launcher/Launcher.h"

#include <Lamp/Core/Application.h>
#include <Lamp/EntryPoint.h>

class LauncherApp : public Lamp::Application
{
public:
	LauncherApp(const Lamp::ApplicationInfo& appInfo)
		: Lamp::Application(appInfo)
	{
		PushLayer(new Launcher::LauncherLayer());
	}

private:
};

Lamp::Application* Lamp::CreateApplication()
{
	Lamp::ApplicationInfo info{};
	info.width = 1600;
	info.height = 900;

	return new LauncherApp(info);
}