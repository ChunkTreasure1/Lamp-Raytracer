#pragma once

#include "Lamp/Core/Base.h"

#include <vector>

namespace Lamp
{
	class Hittable;
	class Scene
	{
	public:
		Scene() = default;

		void OnRender();
		void AddObject(Ref<Hittable> object);

	private:
		std::vector<Ref<Hittable>> m_objects;
	};
}