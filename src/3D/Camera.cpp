/******************************************************************************
 * Copyright (c) 2018-2024 openblack developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/openblack/openblack
 *
 * openblack is licensed under the GNU General Public License version 3.
 *******************************************************************************/

#include "Camera.h"

#include <optional>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/spline.hpp>

#include "3D/LandIslandInterface.h"
#include "ECS/Registry.h"
#include "ECS/Systems/DynamicsSystemInterface.h"
#include "Game.h"
#include "Input/GameActionMapInterface.h"
#include "Locator.h"
#include "Windowing/WindowingInterface.h"

using namespace openblack;

Camera::Camera(glm::vec3 position, glm::vec3 rotation)
    : _position(position)
    , _rotation(glm::radians(rotation))
    , _dv(0.0f, 0.0f, 0.0f)
    , _dwv(0.0f, 0.0f, 0.0f)
    , _dsv(0.0f, 0.0f, 0.0f)
    , _ddv(0.0f, 0.0f, 0.0f)
    , _duv(0.0f, 0.0f, 0.0f)
    , _drv(0.0f, 0.0f, 0.0f)
    , _projectionMatrix(1.0f)
    , _velocity(0.0f, 0.0f, 0.0f)
    , _hVelocity(0.0f, 0.0f, 0.0f)
    , _rotVelocity(0.0f, 0.0f, 0.0f)
    , _accelFactor(0.001f)
    , _movementSpeed(1.5f)
    , _maxMovementSpeed(0.005f)
    , _maxRotationSpeed(0.005f)
    , _lmouseIsDown(false)
    , _mmouseIsDown(false)
    , _mouseIsMoving(false)
    , _mouseFirstClick(0, 0)
    , _shiftHeld(false)
    , _handScreenVec(0)
    , _handDragMult(0.0f)
{
	FlyInit();
}

glm::mat4 Camera::GetRotationMatrix() const
{
	return glm::eulerAngleZXY(_rotation.z, _rotation.x, _rotation.y);
}

glm::mat4 Camera::GetViewMatrix() const
{
	return GetRotationMatrix() * glm::translate(glm::mat4(1.0f), -_position);
}

glm::mat4 Camera::GetViewProjectionMatrix() const
{
	return GetProjectionMatrix() * GetViewMatrix();
}

std::optional<ecs::components::Transform> Camera::RaycastMouseToLand()
{
	// get the hit by raycasting to the land down via the mouse
	ecs::components::Transform intersectionTransform;
	float intersectDistance = 0.0f;
	const auto windowSize = Locator::windowing::value().GetSize();
	glm::vec3 rayOrigin;
	glm::vec3 rayDirection;
	const auto mouseVec = Locator::gameActionSystem::value().GetMousePosition();
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

void Camera::FlyInit()
{
	// call to initialise a new flight. eventually may be refactored out to a separate file.
	_flyInProgress = false;
	_flyDist = 0.0f;
	_flyProgress = 1.0f;
	_flySpeed = 0.5f;
	_flyStartAngle = 10.0f;
	_flyEndAngle = 30.0f;
	_flyThreshold = 50.0f;
	_flyFromPos = glm::vec3(0.0f, 0.0f, 0.0f);
	_flyFromTan = glm::vec3(0.0f, 0.0f, 0.0f);
	_flyToPos = glm::vec3(0.0f, 0.0f, 0.0f);
	_flyToNorm = glm::vec3(0.0f, 0.0f, 0.0f);
	_flyToTan = glm::vec3(0.0f, 0.0f, 0.0f);
	_flyPrevPos = glm::vec3(0.0f, 0.0f, 0.0f);
}

void Camera::StartFlight()
{
	// Call to start the camera "in-flight"
	_flyInProgress = true;
	_flyProgress = 0.0f;
}

void Camera::ResetVelocities()
{
	_dv = glm::vec3(0.0f, 0.0f, 0.0f);
	_drv = glm::vec3(0.0f, 0.0f, 0.0f);
	_dwv = glm::vec3(0.0f, 0.0f, 0.0f);
	_dsv = glm::vec3(0.0f, 0.0f, 0.0f);
	_ddv = glm::vec3(0.0f, 0.0f, 0.0f);
	_duv = glm::vec3(0.0f, 0.0f, 0.0f);
}

void Camera::SetProjectionMatrixPerspective(float xFov, float aspect, float nearClip, float farClip)
{
	const float yFov = (glm::atan(glm::tan(glm::radians(xFov) / 2.0f)) / aspect) * 2.0f;
	const float h = 1.0f / glm::tan(yFov * 0.5f);
	const float w = h / aspect;
	const float a = nearClip / (farClip - nearClip);
	const float b = (nearClip * farClip) / (farClip - nearClip);
	_projectionMatrix = glm::mat4x4(w, 0.f, 0.f, 0.f, 0.f, h, 0.f, 0.f, 0.f, 0.f, a, 1.f, 0.f, 0.f, b, 0.f);
}

glm::vec3 Camera::GetForward() const
{
	// Forward is +1 in openblack but is -1 in OpenGL
	glm::mat3 mRotation = glm::transpose(GetRotationMatrix());
	return mRotation * glm::vec3(0, 0, 1);
}

glm::vec3 Camera::GetRight() const
{
	glm::mat3 mRotation = glm::transpose(GetRotationMatrix());
	return mRotation * glm::vec3(1, 0, 0);
}

glm::vec3 Camera::GetUp() const
{
	glm::mat3 mRotation = glm::transpose(GetRotationMatrix());
	return mRotation * glm::vec3(0, 1, 0);
}

std::unique_ptr<Camera> Camera::Reflect(const glm::vec4& relectionPlane) const
{
	auto reflectionCamera = std::make_unique<ReflectionCamera>(_position, glm::degrees(_rotation), relectionPlane);
	reflectionCamera->SetProjectionMatrix(_projectionMatrix);

	return reflectionCamera;
}

void Camera::DeprojectScreenToWorld(glm::ivec2 screenPosition, glm::ivec2 screenSize, glm::vec3& outWorldOrigin,
                                    glm::vec3& outWorldDirection)
{
	const float normalizedX = static_cast<float>(screenPosition.x) / static_cast<float>(screenSize.x);
	const float normalizedY = static_cast<float>(screenPosition.y) / static_cast<float>(screenSize.y);

	const float screenSpaceX = (normalizedX - 0.5f) * 2.0f;
	const float screenSpaceY = ((1.0f - normalizedY) - 0.5f) * 2.0f;

	// The start of the ray trace is defined to be at mousex,mousey,1 in
	// projection space (z=0 is near, z=1 is far - this gives us better
	// precision) To get the direction of the ray trace we need to use any z
	// between the near and the far plane, so let's use (mousex, mousey, 0.5)
	const glm::vec4 rayStartProjectionSpace = glm::vec4(screenSpaceX, screenSpaceY, 1.0f, 1.0f);
	const glm::vec4 rayEndProjectionSpace = glm::vec4(screenSpaceX, screenSpaceY, 0.5f, 1.0f);

	// Calculate our inverse view projection matrix
	glm::mat4 inverseViewProj = glm::inverse(GetViewProjectionMatrix());

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

void Camera::HandleActions()
{
	const auto& actionSystem = Locator::gameActionSystem::value();

	if (actionSystem.Get(input::UnbindableActionMap::DOUBLE_CLICK))
	{
		const glm::ivec2 mousePosition = actionSystem.GetMousePosition();
		const float dist = glm::distance(glm::vec2(mousePosition), glm::vec2(_mouseFirstClick));
		// fly to double click location.
		if (dist < 10.0f)
		{
			if (auto hit = RaycastMouseToLand())
			{
				// stop all current movements
				ResetVelocities();

				_flyToNorm = hit->rotation * glm::vec3(0.0f, 1.0f, 0.0f);
				auto normXZ = glm::normalize(_flyToNorm * glm::vec3(1.0f, 0.01f, 1.0f));
				_flyInProgress = true;
				_flyProgress = 0.0f;
				_flyFromPos = _position;
				_flyPrevPos = _flyFromPos;
				_flyDist = glm::length(hit->position - _flyFromPos);
				auto vecToCam = glm::normalize(_position - hit->position);
				_flyToPos = hit->position + (normXZ + vecToCam * 4.0f) / 5.0f * std::max(20.0f, _flyDist * 0.15f);
				_flyFromTan = glm::normalize(GetForward() * glm::vec3(1.0f, 0.0f, 1.0f)) * _flyDist * 0.4f;
				_flyToTan =
				    glm::normalize(-(_flyToNorm * 9.0f + vecToCam) / 10.0f * glm::vec3(1.0f, 0.0f, 1.0f)) * _flyDist * 0.4f;
				if (_position.y < _flyThreshold) // if the camera is low to the ground aim the path up before coming back down
				{
					_flyFromTan += glm::vec3(0.0f, 1.0f, 0.0f) * _flyDist * 0.4f;
					_flyToTan += glm::vec3(0.0f, -1.0f, 0.0f) * _flyDist * 0.4f;
				}
			}
		}
	}

	if (actionSystem.GetChanged(input::BindableActionMap::MOVE) && !actionSystem.Get(input::BindableActionMap::MOVE))
	{
		_lmouseIsDown = false;
		_mouseFirstClick = actionSystem.GetMousePosition();
	}
	else if (actionSystem.GetChanged(input::BindableActionMap::MOVE) && actionSystem.Get(input::BindableActionMap::MOVE))
	{
		if (!_lmouseIsDown)
		{
			ResetVelocities();
		}
		if (!_mmouseIsDown)
		{
			_lmouseIsDown = true;
		}
	}
	if (actionSystem.GetChanged(input::BindableActionMap::ROTATE_AROUND_MOUSE_ON) &&
	    !actionSystem.Get(input::BindableActionMap::ROTATE_AROUND_MOUSE_ON))
	{
		_mmouseIsDown = false;
	}
	else if (actionSystem.GetChanged(input::BindableActionMap::ROTATE_AROUND_MOUSE_ON) &&
	         actionSystem.Get(input::BindableActionMap::ROTATE_AROUND_MOUSE_ON))
	{
		_mmouseIsDown = true;
		_lmouseIsDown = false;
		if (!_mmouseIsDown)
		{
			ResetVelocities();
		}
	}

	if (actionSystem.GetAny(input::BindableActionMap::ZOOM_IN, input::BindableActionMap::ZOOM_OUT)) // scroll up or down
	{
		auto movementSpeed = _movementSpeed * 4 * glm::smoothstep(0.1f, 1.0f, _position.y * 0.01f) * glm::log(_position.y + 1);
		_flyInProgress = false;
		const auto direction = actionSystem.Get(input::BindableActionMap::ZOOM_IN) ? 10.0f : -10.0f;
		auto dist = 9999.0f;
		const auto& dynamicsSystem = Locator::dynamicsSystem::value();
		if (auto hit = dynamicsSystem.RayCastClosestHit(_position, GetForward(), 1e10f))
		{
			dist = glm::length(hit->first.position - _position);
		}
		auto amount = (((movementSpeed * direction * _maxMovementSpeed) - _velocity.z) * _accelFactor);
		if (actionSystem.Get(input::BindableActionMap::ZOOM_IN)) // scrolling in
		{
			if (dist > 40.0f) // if the cam is far from the ground
			{
				_velocity.z += amount;
			}
			else // if the cam is just over the ground
			{
				if (_rotation.x > glm::radians(-60.0f)) // rotation greater than -60 degrees
				{
					_rotVelocity.x += (((-direction * 4.0f * _maxRotationSpeed) - _rotVelocity.x) * _accelFactor);
				}
			}
		}
		else // scrolling out
		{
			if (dist <= 40.0f && _rotation.x < glm::radians(-50.0f))
			{
				_rotVelocity.x += (((-direction * 4.0f * _maxRotationSpeed) - _rotVelocity.x) * _accelFactor);
			}
			else
			{
				_velocity.z += amount;
			}
		}
	}

	if (actionSystem.GetMouseDelta() != glm::zero<glm::ivec2>())
	{
		if (actionSystem.Get(input::BindableActionMap::ROTATE_AROUND_MOUSE_ON))
		{
			const auto delta = actionSystem.GetMouseDelta();
			const auto handPositions = actionSystem.GetHandPositions();
			if (_shiftHeld) // Holding down the middle mouse button and shift enables FPV camera rotation.
			{
				glm::vec3 rot = GetRotation();
				rot.y -= delta.x * glm::radians(0.1f);
				rot.x -= delta.y * glm::radians(0.1f);
				SetRotation(rot);
			}
			// Holding down the middle mouse button without shift enables hand orbit camera rotation.
			else if (handPositions[0].has_value() || handPositions[1].has_value())
			{
				auto handPos = handPositions[0].value_or(handPositions[1].value_or(glm::zero<glm::vec3>()));

				auto handDist = glm::length2(handPos - _position);
				if (handDist > 250000.0f) // if hand is more than 500 away (500^2)
				{
					handPos = _position + GetForward() * 500.0f; // orbit around a point 500 away from cam
				}
				const auto screenSize = Locator::windowing::value().GetSize();
				const float yaw = delta.x * (glm::two_pi<float>() / screenSize.x);
				float pitch = delta.y * (glm::pi<float>() / screenSize.y);

				// limit orbit cam by cam rotation in x
				if (pitch > 0.0f)
				{
					auto pitchMult = glm::smoothstep(0.f, 0.1f, _rotation.x + glm::radians(60.0f));
					pitch *= pitchMult;
				}

				_rotation.x -= pitch;
				_rotation.y -= yaw;

				glm::mat4 rotationMatrixX(1.0f);
				rotationMatrixX = glm::rotate(rotationMatrixX, yaw, glm::vec3(0.0f, 1.0f, 0.0f));
				_position = (rotationMatrixX * glm::vec4(_position - handPos, 0.0f)) + glm::vec4(handPos, 0.0f);

				glm::mat4 rotationMatrixY(1.0f);
				rotationMatrixY = glm::rotate(rotationMatrixY, pitch, GetRight());
				_position = (rotationMatrixY * glm::vec4(_position - handPos, 0.0f)) + glm::vec4(handPos, 0.0f);
				_position.y = (_position.y < 15.0f) ? 15.0f : _position.y;
			}
		}
		else if (actionSystem.Get(input::BindableActionMap::MOVE))
		{
			_mouseIsMoving = true;
		}
	}

	auto movementSpeed = _movementSpeed * std::max(_position.y * 0.01f, 0.0f) + 1.0f;

	// ignore all repeated keys
	if (actionSystem.GetRepeatAll(input::BindableActionMap::ROTATE_ON, input::BindableActionMap::MOVE_LEFT,
	                              input::BindableActionMap::MOVE_RIGHT, input::BindableActionMap::MOVE_FORWARDS,
	                              input::BindableActionMap::MOVE_BACKWARDS, input::BindableActionMap::ROTATE_LEFT,
	                              input::BindableActionMap::ROTATE_RIGHT, input::BindableActionMap::TILT_UP,
	                              input::BindableActionMap::TILT_DOWN))
	{
		return;
	}

	// stop Fly if any movement keys are pressed down
	if (actionSystem.GetAny(input::BindableActionMap::ROTATE_ON, input::BindableActionMap::MOVE_LEFT,
	                        input::BindableActionMap::MOVE_RIGHT, input::BindableActionMap::MOVE_FORWARDS,
	                        input::BindableActionMap::MOVE_BACKWARDS, input::BindableActionMap::ROTATE_LEFT,
	                        input::BindableActionMap::ROTATE_RIGHT, input::BindableActionMap::TILT_UP,
	                        input::BindableActionMap::TILT_DOWN))
	{
		_flyInProgress = false;
	}

	if (_mmouseIsDown || _lmouseIsDown)
	{
		ResetVelocities();
	}
	else
	{
		_shiftHeld = actionSystem.Get(input::BindableActionMap::ROTATE_ON);
		if (actionSystem.GetChanged(input::BindableActionMap::MOVE_FORWARDS))
		{
			if (actionSystem.Get(input::BindableActionMap::MOVE_FORWARDS))
			{
				glm::vec3 temp = GetForward() * glm::vec3(1.f, 0.f, 1.f);
				glm::mat3 mRotation = glm::transpose(GetRotationMatrix());
				temp = glm::normalize(temp) * mRotation * movementSpeed;
				_dv += temp;
				_dwv = temp;
			}
			else
			{
				_dv -= _dwv;
				_dwv = glm::vec3(0.0f, 0.0f, 0.0f);
			}
		}
		else if (actionSystem.GetChanged(input::BindableActionMap::MOVE_BACKWARDS))
		{
			if (actionSystem.Get(input::BindableActionMap::MOVE_BACKWARDS))
			{
				glm::vec3 temp = -GetForward() * glm::vec3(1.f, 0.f, 1.f);
				glm::mat3 mRotation = glm::transpose(GetRotationMatrix());
				temp = glm::normalize(temp) * mRotation * movementSpeed;
				_dv += temp;
				_dsv = temp;
			}
			else
			{
				_dv -= _dsv;
				_dsv = glm::vec3(0.0f, 0.0f, 0.0f);
			}
		}
		else if (actionSystem.GetChanged(input::BindableActionMap::MOVE_LEFT))
		{
			_dv.x += actionSystem.Get(input::BindableActionMap::MOVE_LEFT) ? -movementSpeed : -_dv.x;
		}
		else if (actionSystem.GetChanged(input::BindableActionMap::MOVE_RIGHT))
		{
			_dv.x += actionSystem.Get(input::BindableActionMap::MOVE_RIGHT) ? movementSpeed : -_dv.x;
		}
		else if (actionSystem.GetChanged(input::BindableActionMap::ROTATE_LEFT))
		{
			_drv.y += actionSystem.Get(input::BindableActionMap::ROTATE_LEFT) ? _movementSpeed : -_drv.y;
		}
		else if (actionSystem.GetChanged(input::BindableActionMap::ROTATE_RIGHT))
		{
			_drv.y += actionSystem.Get(input::BindableActionMap::ROTATE_RIGHT) ? -_movementSpeed : -_drv.y;
		}
		else if (actionSystem.GetChanged(input::BindableActionMap::TILT_UP))
		{
			_drv.x += actionSystem.Get(input::BindableActionMap::TILT_UP) ? _movementSpeed : -_drv.x;
		}
		else if (actionSystem.GetChanged(input::BindableActionMap::TILT_DOWN))
		{
			_drv.x += actionSystem.Get(input::BindableActionMap::TILT_DOWN) ? -_movementSpeed : -_drv.x;
		}
		else if (actionSystem.GetChanged(input::BindableActionMap::ZOOM_TO_TEMPLE))
		{
			if (actionSystem.Get(input::BindableActionMap::ZOOM_TO_TEMPLE))
			{
				movementSpeed =
				    _movementSpeed * 4 * glm::smoothstep(0.1f, 1.0f, _position.y * 0.01f) * glm::log(_position.y + 1);
				glm::vec3 temp = glm::vec3(0.f, 1.f, 0.f);
				const glm::mat3 mRotation = glm::transpose(GetRotationMatrix());
				if (_shiftHeld)
				{
					temp = glm::normalize(-temp) * mRotation * movementSpeed;
					_dv += temp;
					_ddv = temp;
					_flyInProgress = false;
				}
				else
				{
					temp = glm::normalize(temp) * mRotation * movementSpeed;
					_dv += temp;
					_duv = temp;
					_flyInProgress = false;
				}
			}
			else
			{
				_dv -= _ddv;
				_ddv = glm::vec3(0.0f, 0.0f, 0.0f);
				_dv -= _duv;
				_duv = glm::vec3(0.0f, 0.0f, 0.0f);
			}
		}
	}
}

void Camera::Update(std::chrono::microseconds dt)
{
	auto airResistance = .92f; // reduced to make more floaty
	auto fdt = static_cast<float>(dt.count());
	glm::mat3 rotation = glm::transpose(GetViewMatrix());

	// deal with hand pulling camera around
	float worldHandDist = 0.0f;
	const auto size = Locator::windowing::value().GetSize();
	const auto& actionSystem = Locator::gameActionSystem::value();
	const auto handPositions = actionSystem.GetHandPositions();
	if (_lmouseIsDown && (handPositions[0].has_value() || handPositions[1].has_value())) // drag camera using hand
	{
		// get hand transform and project to screen coordinates
		auto handPos = handPositions[0].value_or(handPositions[1].value_or(glm::zero<glm::vec3>()));
		const glm::vec3 handOffset(0, 1.5f, 0);
		glm::vec3 handToScreen;
		auto viewport = glm::vec4(0, 0, size.x, size.y);
		auto hit = RaycastMouseToLand();
		if (hit)
		{
			handPos -= handOffset * glm::transpose(hit->rotation);
		}
		if (ProjectWorldToScreen(handPos, viewport, handToScreen) && hit)
		{
			// calculate distance between hand and mouse in screen coordinates
			const auto mousePosition = actionSystem.GetMousePosition();
			auto handScreenCoordinates = glm::ivec2(handToScreen);
			handScreenCoordinates.y = size.y - handScreenCoordinates.y;
			_handScreenVec = static_cast<glm::ivec2>(mousePosition) - handScreenCoordinates;
			_handDragMult = glm::length(glm::vec2(_handScreenVec));
			worldHandDist = glm::length(hit->position - handPos);
			_handDragMult /= size.y;
		}
		// still on screen but did not hit land
		else if (!hit)
		{
			_handDragMult -= 0.002f; // slow down movement
		}
		else // if hand is off-screen, culled or behind camera.
		{
			_hVelocity = glm::vec3(0.f);
		}
	}
	else
	{
		_handDragMult = (_handDragMult > 0.0f) ? _handDragMult * 0.96f
		                                       : _handDragMult; // if let go of mouse button slow down hand movement.
		_hVelocity = glm::vec3(0.f);
	}

	if (_handDragMult > 0.0f)
	{
		auto momentum = _position.y / 300;
		auto forward =
		    glm::normalize(GetForward() * glm::vec3(1.f, 0.f, 1.f)) * static_cast<float>(_handScreenVec.y * momentum);
		auto right = GetRight() * -static_cast<float>(_handScreenVec.x * momentum);
		auto futurePosition = _position + forward + right;
		auto logPosY = glm::log(_position.y + 1.0f);
		auto handVelHeightMult = logPosY * logPosY;
		glm::vec3 vecTo = futurePosition - _position;
		if (vecTo != glm::vec3(0.0f))
		{
			vecTo = glm::min(glm::normalize(vecTo) * handVelHeightMult, glm::vec3(5.0f));
		}
		glm::mat3 mRotation = glm::transpose(GetRotationMatrix());
		_hVelocity += vecTo * mRotation * 0.00005f;
		if (GetForward().y > 0.0f) // camera is pointing upwards
		{
			if (_handScreenVec.y > 0.0f) // if the move direction is also up
			{
				_hVelocity.y += 0.0005f; // move cam up a little
			}
			else
			{
				_hVelocity.y -= 0.0005f; // move cam down a little
			}
		}
	}

	if (worldHandDist > 2000.0f)
	{
		// speed up movement for really distant hand pulls.
		_handDragMult += 0.001f;
	}
	else if (worldHandDist < 2000.0f && worldHandDist > 0.0f && _handDragMult >= 0.1f)
	{
		// slow movement for less distant hand pulls.
		_handDragMult *= 0.98f;
	}
	else if (worldHandDist < 0.0f)
	{
		_handDragMult = 0.0f;
	}

	if (_flyInProgress)
	{
		_position = glm::hermite(_flyFromPos, _flyFromTan, _flyToPos, _flyToTan, glm::smoothstep(0.0f, 1.0f, _flyProgress));

		// Check if there are obstacles in the way, if there are fly over them
		const auto& dynamicsSystem = Locator::dynamicsSystem::value();
		if (auto obst = dynamicsSystem.RayCastClosestHit(_position - glm::vec3(0.0f, 20.0f, 0.0f),
		                                                 glm::normalize((_flyToPos - glm::vec3(0.0f, 20.0f, 0.0f)) - _position),
		                                                 glm::length(_flyToPos - _position) + 10.0f))
		{
			auto closest = obst->first;
			auto dist = glm::length(_flyToPos - closest.position);
			if (dist > 60.0f)
			{
				_flyFromTan += glm::vec3(0.0f, 1.0f, 0.0f) * _flyDist * 0.006f;
				_flyToTan += glm::vec3(0.0f, -1.0f, 0.0f) * _flyDist * 0.006f;
			}
		}

		if (_flyPrevPos != _position && _flyProgress > 0.0f)
		{
			auto tangentY = glm::normalize((_position - _flyPrevPos) * glm::vec3(1.0f, 0.0f, 1.0f));
			auto currDirY = glm::normalize(GetForward() * glm::vec3(1.0f, 0.0f, 1.0f));
			float rotY =
			    atan2f(glm::dot(glm::cross(tangentY, currDirY), glm::vec3(0.0f, 1.0f, 1.0f)), glm::dot(currDirY, tangentY));
			_drv.y = rotY * 5.0f * ((_flyProgress * _flyProgress) * -1.0f + 1.0f);
		}
		if (_flyProgress >= 1.0f)
		{
			_flyInProgress = false;
			_drv = glm::vec3(0.0f);
		}
		else if (_flyProgress >= 0.5f && glm::dot(_flyToNorm, glm::vec3(0.0f, 1.0f, 0.0f)) > 0.8f) // rotate cam down
		{
			float angleDown = -glm::radians(_flyEndAngle) - _rotation.x;
			_drv.x = angleDown * 2.0f * (((_flyProgress * 2 - 2) * (_flyProgress * 2 - 2)) * -1.0f + 1.0f);
		}
		else if (_flyProgress <= 0.5f && _position.y < _flyThreshold) // rotate cam up
		{
			float angleUp = glm::radians(_flyStartAngle) - _rotation.x;
			_drv.x = angleUp * 0.5f * (((_flyProgress * 2 - 1) * (_flyProgress * 2 - 1)) * -1.0f + 1.0f);
		}
		_flyPrevPos = _position;
		_flyProgress += _flySpeed * 0.000001f * fdt;
	}
	else if (!_flyInProgress && _flyProgress < 1.0f) // player aborted a Fly early
	{
		_flyProgress = 1.0f;
		_drv = glm::vec3(0.0f);
	}
	else
	{
		_velocity += (((_dv * _maxMovementSpeed) - _velocity) * _accelFactor);
		_hVelocity *= _handDragMult;
		_position += rotation * (_velocity + _hVelocity) * fdt;
	}
	const auto& land = Locator::terrainSystem::value();

	auto height = land.GetHeightAt(glm::vec2(_position.x + 5, _position.z + 5));
	_position.y =
	    (_position.y < height + 13.0f) ? height + 13.0f : _position.y; // stop the camera from going below ground level.
	_rotVelocity += (((_drv * _maxRotationSpeed) - _rotVelocity) * _accelFactor);
	glm::vec3 rot = GetRotation();
	rot += glm::radians(_rotVelocity * fdt);
	rot.x = glm::max(rot.x, glm::radians(-70.0f)); // limit cam rotation in x
	SetRotation(rot);

	_velocity *= airResistance;
	_rotVelocity *= airResistance;
	_mouseIsMoving = false;
}

glm::mat4 ReflectionCamera::GetViewMatrix() const
{
	glm::mat4 mRotation = GetRotationMatrix();
	glm::mat4 mView = mRotation * glm::translate(glm::mat4(1.0f), -_position);

	// M''camera = Mreflection * Mcamera * Mflip
	glm::mat4 reflectionMatrix;
	ReflectMatrix(reflectionMatrix, _reflectionPlane);

	return mView * reflectionMatrix;
}

/*
              | 1-2Nx2   -2NxNy  -2NxNz  -2NxD |
Mreflection = |  -2NxNy 1-2Ny2   -2NyNz  -2NyD |
              |  -2NxNz  -2NyNz 1-2Nz2   -2NzD |
              |    0       0       0       1   |
*/
void ReflectionCamera::ReflectMatrix(glm::mat4& m, const glm::vec4& plane) const
{
	m[0][0] = (1.0f - 2.0f * plane[0] * plane[0]);
	m[1][0] = (-2.0f * plane[0] * plane[1]);
	m[2][0] = (-2.0f * plane[0] * plane[2]);
	m[3][0] = (-2.0f * plane[3] * plane[0]);

	m[0][1] = (-2.0f * plane[1] * plane[0]);
	m[1][1] = (1.0f - 2.0f * plane[1] * plane[1]);
	m[2][1] = (-2.0f * plane[1] * plane[2]);
	m[3][1] = (-2.0f * plane[3] * plane[1]);

	m[0][2] = (-2.0f * plane[2] * plane[0]);
	m[1][2] = (-2.0f * plane[2] * plane[1]);
	m[2][2] = (1.0f - 2.0f * plane[2] * plane[2]);
	m[3][2] = (-2.0f * plane[3] * plane[2]);

	m[0][3] = 0.0f;
	m[1][3] = 0.0f;
	m[2][3] = 0.0f;
	m[3][3] = 1.0f;
}
