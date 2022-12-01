#include "lppch.h"
#include "Scene.h"

#include "Lamp/Rendering/Renderer.h"

namespace Lamp
{
	void Scene::OnRender()
	{
		for (const auto& obj : m_objects)
		{
			Renderer::Submit(obj);
		}
	}

	void Scene::AddObject(Ref<Hittable> object)
	{
		m_objects.emplace_back(object);
	}
}