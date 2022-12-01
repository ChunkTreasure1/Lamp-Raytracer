#pragma once

#include <glm/glm.hpp>

namespace Lamp
{
	struct Ray
	{
		Ray() = default;
		Ray(const glm::vec3& aOrigin, const glm::vec3& aDirection)
			: origin(aOrigin), direction(aDirection)
		{}

		inline const glm::vec3 GetAt(float t) const { return origin + direction * t; }

		glm::vec3 origin;
		glm::vec3 direction;
	};

	struct RaycastHit
	{
		glm::vec3 position;
		glm::vec3 normal;
		float distance;
		bool frontFace;

		inline void SetFaceNormal(const Ray& ray, const glm::vec3& outwardNormal)
		{
			frontFace = glm::dot(ray.direction, outwardNormal) < 0.f;
			normal = frontFace ? outwardNormal : -outwardNormal;
		}
	};
}