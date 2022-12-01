#pragma once

#include "Lamp/Math/Ray.h"

namespace Lamp
{
	class Hittable
	{
	public:
		virtual bool HitTest(const Ray& ray, const float minT, const float maxT, RaycastHit& hit) const = 0;
	};
}