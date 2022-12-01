#include "lppch.h"
#include "Camera.h"

namespace Lamp
{
	Camera::Camera(float fov, float aspect, float nearPlane, float farPlane)
		: m_fieldOfView(fov), m_aspectRatio(aspect), m_nearPlane(nearPlane), m_farPlane(farPlane)
	{
		m_projectionMatrix = glm::perspective(glm::radians(m_fieldOfView), aspect, m_nearPlane, m_farPlane);
		m_viewMatrix = glm::mat4(1.f);
	}

	Camera::Camera(float left, float right, float bottom, float top, float nearPlane, float farPlane)
		: m_nearPlane(nearPlane), m_farPlane(farPlane)
	{
		m_projectionMatrix = glm::ortho(left, right, bottom, top, -1.f, 1.f);
		m_viewMatrix = glm::mat4(1.f);
	}

	void Camera::SetPerspectiveProjection(float fov, float aspect, float nearPlane, float farPlane)
	{
		m_fieldOfView = fov;
		m_aspectRatio = aspect;
		m_nearPlane = nearPlane;
		m_farPlane = farPlane;

		m_projectionMatrix = glm::perspective(glm::radians(m_fieldOfView), m_aspectRatio, m_nearPlane, m_farPlane);
	}

	void Camera::SetOrthographicProjection(float left, float right, float bottom, float top)
	{
		m_projectionMatrix = glm::ortho(left, right, bottom, top, -1.f, 1.f);
	}

	void Camera::GenerateRayDirections(uint32_t width, uint32_t height)
	{
		m_rayDirections.clear();
		m_rayDirections.resize(width * height);
		
		for (uint32_t x = 0; x < width; x++)
		{
			for (uint32_t y = 0; y < height; y++)
			{
				m_rayDirections[x + y * width] = ScreenToWorldRay({ x, y }, { width, height });
			}
		}
	}

	const glm::vec3& Camera::GetRayDirectionAt(uint32_t index)
	{
		return m_rayDirections.at(index);
	}

	const glm::vec3 Camera::ScreenToWorldRay(const glm::vec2& someCoords, const glm::vec2& aSize)
	{
		LP_PROFILE_FUNCTION();

		float x = (someCoords.x / aSize.x) * 2.f - 1.f;
		float y = (someCoords.y / aSize.y) * 2.f - 1.f;

		const glm::mat4 matInv = glm::inverse(m_projectionMatrix * m_viewMatrix);

		glm::vec4 rayOrigin = matInv * glm::vec4(x, -y, 0, 1);
		glm::vec4 rayEnd = matInv * glm::vec4(x, -y, 1, 1);

		if (rayOrigin.w == 0 || rayEnd.w == 0)
		{
			return { 0, 0, 0 };
		}

		rayOrigin /= rayOrigin.w;
		rayEnd /= rayEnd.w;

		const glm::vec3 rayDir = glm::normalize(rayEnd - rayOrigin);

		return rayDir;
	}

	glm::vec3 Camera::GetUp() const
	{
		return glm::rotate(GetOrientation(), glm::vec3{ 0.f, 1.f, 0.f });
	}

	glm::vec3 Camera::GetRight() const
	{
		return glm::rotate(GetOrientation(), glm::vec3{ 1.f, 0.f, 0.f });
	}

	glm::vec3 Camera::GetForward() const
	{
		return glm::rotate(GetOrientation(), glm::vec3{ 0.f, 0.f, -1.f });
	}

	glm::quat Camera::GetOrientation() const
	{
		return glm::quat(glm::vec3(glm::radians(-m_rotation.y), glm::radians(-m_rotation.x), 0.f));
	}

	void Camera::RecalculateViewMatrix()
	{
		const float yawSign = GetUp().y < 0 ? -1.0f : 1.0f;

		const glm::vec3 lookAt = m_position + GetForward();
		m_viewMatrix = glm::lookAt(m_position, lookAt, glm::vec3(0.f, yawSign, 0.f));
	}
}