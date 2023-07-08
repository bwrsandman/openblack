/*****************************************************************************
 * Copyright (c) 2018-2023 openblack developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/openblack/openblack
 *
 * openblack is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#include "Camera.h"

#include <optional>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/intersect.hpp>

#include "3D/LandIsland.h"
#include "ECS/Registry.h"
#include "ECS/Systems/DynamicsSystemInterface.h"
#include "Game.h"
#include "Input/GameActionMapInterface.h"
#include "Locator.h"
#include "ReflectionXZCamera.h"

using namespace openblack;

Camera::Camera()
    : _model(CameraModel::CreateModel(CameraModel::Model::DefaultWorld))
{
}

glm::mat4 Camera::GetRotationMatrix() const
{
	return static_cast<glm::mat3>(glm::transpose(GetViewMatrix()));
}

glm::mat4 Camera::GetViewMatrix() const
{
	// Invert the camera's rotation (transposed) and position (negated) to get the view matrix.
	return glm::lookAt(_position, _focus, glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::mat4 Camera::GetViewProjectionMatrix() const
{
	return GetProjectionMatrix() * GetViewMatrix();
}

std::optional<ecs::components::Transform> Camera::RaycastMouseToLand() const
{
	// get the hit by raycasting to the land down via the mouse
	ecs::components::Transform intersectionTransform;
	float intersectDistance = 0.0f;
	const auto windowSize = Game::Instance()->GetWindow()->GetSize();
	glm::vec3 rayOrigin;
	glm::vec3 rayDirection;
	const glm::ivec2 mouseVec = Locator::gameActionSystem::value().GetMousePosition();
	DeprojectScreenToWorld(mouseVec, windowSize, rayOrigin, rayDirection);
	const auto& dynamicsSystem = Locator::dynamicsSystem::value();
	if (auto hit = dynamicsSystem.RayCastClosestHit(rayOrigin, rayDirection, 1e10f))
	{
		intersectionTransform = hit->first;
		return std::make_optional(intersectionTransform);
	}
	if (glm::intersectRayPlane(rayOrigin, rayDirection, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f),
	                           intersectDistance))
	{
		intersectionTransform.position = rayOrigin + rayDirection * intersectDistance;
		intersectionTransform.rotation = glm::mat3(1.0f);
		return std::make_optional(intersectionTransform);
	}
	return std::nullopt;
}

Camera& Camera::SetProjectionMatrixPerspective(float xFov, float aspect, float nearClip, float farClip)
{
	auto yFov = (glm::atan(glm::tan(glm::radians(xFov) / 2.0f)) / aspect) * 2.0f;
	_projectionMatrix = glm::perspective(yFov, aspect, nearClip, farClip);

	return *this;
}

glm::vec3 Camera::GetForward() const
{
	// Forward is +1 in openblack but is -1 in OpenGL
	return static_cast<glm::mat3>(GetRotationMatrix()) * glm::vec3(0.0f, 0.0f, 1.0f);
}

glm::vec3 Camera::GetRight() const
{
	return static_cast<glm::mat3>(GetRotationMatrix()) * glm::vec3(1.0f, 0.0f, 0.0f);
}

glm::vec3 Camera::GetUp() const
{
	return static_cast<glm::mat3>(GetRotationMatrix()) * glm::vec3(0.0f, 1.0f, 0.0f);
}

std::unique_ptr<Camera> Camera::Reflect() const
{
	// TODO(bwrsandman): The copy to reflection camera has way too much of Camera including model which is useless
	//                   This also touches on other cameras such as the citadel camera which use a different kind of model
	auto reflectionCamera = std::make_unique<ReflectionXZCamera>();
	(*reflectionCamera).SetPosition(_position).SetFocus(_focus).SetProjectionMatrix(_projectionMatrix);
	return reflectionCamera;
}

void Camera::DeprojectScreenToWorld(glm::ivec2 screenPosition, glm::ivec2 screenSize, glm::vec3& outWorldOrigin,
                                    glm::vec3& outWorldDirection) const
{
	const float normalizedX = static_cast<float>(screenPosition.x) / static_cast<float>(screenSize.x);
	const float normalizedY = static_cast<float>(screenPosition.y) / static_cast<float>(screenSize.y);

	const float screenSpaceX = (normalizedX - 0.5f) * 2.0f;
	const float screenSpaceY = ((1.0f - normalizedY) - 0.5f) * 2.0f;

	// The start of the ray trace is defined to be at mousex,mousey,1 in
	// projection space (z=0 is near, z=1 is far - this gives us better
	// precision) To get the direction of the ray trace we need to use any z
	// between the near and the far plane, so let's use (mousex, mousey, 0.5)
	const glm::vec4 rayStartProjectionSpace = glm::vec4(screenSpaceX, screenSpaceY, 0.0f, 1.0f);
	const glm::vec4 rayEndProjectionSpace = glm::vec4(screenSpaceX, screenSpaceY, 0.5f, 1.0f);

	// Calculate our inverse view projection matrix
	auto inverseViewProj = glm::inverse(GetViewProjectionMatrix());

	// Get our homogeneous coordinates for our start and end ray positions
	const glm::vec4 hgRayStartWorldSpace = inverseViewProj * rayStartProjectionSpace;
	const glm::vec4 hgRayEndWorldSpace = inverseViewProj * rayEndProjectionSpace;

	glm::vec3 rayStartWorldSpace(hgRayStartWorldSpace.x, hgRayStartWorldSpace.y, hgRayStartWorldSpace.z);
	glm::vec3 rayEndWorldSpace(hgRayEndWorldSpace.x, hgRayEndWorldSpace.y, hgRayEndWorldSpace.z);

	// divide vectors by W to undo any projection and get the 3-space coord
	if (hgRayStartWorldSpace.w != 0.0f)
	{
		rayStartWorldSpace /= hgRayStartWorldSpace.w;
	}

	if (hgRayEndWorldSpace.w != 0.0f)
	{
		rayEndWorldSpace /= hgRayEndWorldSpace.w;
	}

	const glm::vec3 rayDirWorldSpace = glm::normalize(rayEndWorldSpace - rayStartWorldSpace);

	// finally, store the results in the outputs
	outWorldOrigin = rayStartWorldSpace;
	outWorldDirection = rayDirWorldSpace;
}

bool Camera::ProjectWorldToScreen(glm::vec3 worldPosition, glm::vec4 viewport, glm::vec3& outScreenPosition) const
{
	outScreenPosition = glm::project(worldPosition, GetViewMatrix(), GetProjectionMatrix(), viewport);
	if (outScreenPosition.x < viewport.x || outScreenPosition.y < viewport.y || outScreenPosition.x > viewport.z ||
	    outScreenPosition.y > viewport.w)
	{
		return false; // Outside viewport bounds
	}
	if (outScreenPosition.z > 1.0f)
	{
		return false; // Behind Camera
	}
	if (outScreenPosition.z < 0.0f)
	{
		return false; // Clipped
	}
	return true;
}

void Camera::Update(std::chrono::microseconds dt)
{
	_model->Update(dt, *this);

	SetPosition(_model->GetTargetPosition());
	SetFocus(_model->GetTargetFocus());
}

void Camera::HandleActions(std::chrono::microseconds dt)
{
	_model->HandleActions(dt);
}

const glm::mat4& Camera::GetProjectionMatrix() const
{
	return _projectionMatrix;
}

glm::vec3 Camera::GetPosition() const
{
	return _position;
}

glm::vec3 Camera::GetFocus() const
{
	return _focus;
}

glm::vec3 Camera::GetRotation() const
{
	// Extract the yaw, pitch, and roll angles from the rotation matrix
	float yaw, pitch, roll;

	// Use the GLM function `extractEulerAngleYXZ` to get the Euler angles
	glm::extractEulerAngleZYX(GetRotationMatrix(), roll, yaw, pitch);
	// Fix angles to make sure roll is 0, it tends to flip to 180 and -180
	if (roll > glm::radians(90.0f) || roll < glm::radians(-90.0f))
	{
		pitch -= glm::radians(180.0f) * glm::sign(pitch);
		yaw = glm::radians(180.0f) - yaw;
	}
	return glm::vec3(pitch, yaw, 0.0f);
}

glm::vec3 Camera::GetTargetPosition() const
{
	// TODO: Camera should interpolate between current state and target with a modified sigmoid interpolation
	return GetPosition();
}

glm::vec3 Camera::GetTargetFocus() const
{
	// TODO: Camera should interpolate between current state and target with a modified sigmoid interpolation
	return GetFocus();
}

Camera& Camera::SetPosition(const glm::vec3& position)
{
	_position = position;

	return *this;
}

Camera& Camera::SetFocus(const glm::vec3& position)
{
	_focus = position;

	return *this;
}

Camera& Camera::SetProjectionMatrix(const glm::mat4& projection)
{
	_projectionMatrix = projection;

	return *this;
}
