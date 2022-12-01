#pragma once

#include <Lamp/Scene/Hittable.h>

namespace Launcher
{
	class Sphere : public Lamp::Hittable
	{
	public:
		Sphere(const glm::vec3& center, const float radius);
		bool HitTest(const Lamp::Ray& ray, const float minT, const float maxT, Lamp::RaycastHit& hit) const override;
		
	private:
		glm::vec3 m_center;
		float m_radius;
	};
}