#include "Sphere.h"

#include <glm/gtx/norm.hpp>

namespace Launcher
{
	Sphere::Sphere(const glm::vec3& center, const float radius)
		: m_center(center), m_radius(radius)
	{}

	bool Sphere::HitTest(const Lamp::Ray& ray, const float minT, const float maxT, Lamp::RaycastHit& hit) const
	{
		const glm::vec3 oc = ray.origin - m_center;
		const float a = glm::length2(ray.direction);
		const float halfB = glm::dot(oc, ray.direction);
		const float c = glm::length2(oc) - m_radius * m_radius;

		const float discriminant = halfB * halfB - a * c;

		if (discriminant < 0.f)
		{
			return false;
		}

		const float discSqrt = std::sqrt(discriminant);

		// Find nearest root
		float root = (-halfB - discSqrt) / a;
		if (root < minT || root > maxT)
		{
			root = (-halfB + discSqrt) / a;
			if (root < minT || root > maxT)
			{
				return false;
			}
		}

		hit.distance = root;
		hit.position = ray.GetAt(root);
		hit.normal = (hit.position - m_center) / m_radius;

		const glm::vec3 outwardNormal = (hit.position - m_center) / m_radius;
		hit.SetFaceNormal(ray, outwardNormal);

		return true;
	}
}