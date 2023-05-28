/*****************************************************************************
 * Copyright (c) 2018-2023 openblack developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/openblack/openblack
 *
 * openblack is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#include "DefaultWorldCameraModel.h"

#include <glm/gtc/constants.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/vec_swizzle.hpp>
#include <spdlog/spdlog.h>

#include "Camera.h"
#include "Game.h"
#include "Input/GameActionMapInterface.h"
#include "Locator.h"

using namespace openblack;

DefaultWorldCameraModel::DefaultWorldCameraModel() = default;

DefaultWorldCameraModel::~DefaultWorldCameraModel() = default;

void DefaultWorldCameraModel::TiltZoom(glm::vec3& eulerAngles, float scalingFactor)
{
	// TODO: Early out if mouse middle button pressed or left+right pressed

	// Update the camera's yaw if there's significant horizontal movement.
	if (glm::abs(_rotateAroundDelta.y) > glm::epsilon<float>())
	{
		eulerAngles.x += _rotateAroundDelta.y * glm::pi<float>() / Game::Instance()->GetWindow()->GetSize().x;
	}

	// Update the camera's pitch if there's significant vertical movement.
	if (glm::abs(_rotateAroundDelta.x) > glm::epsilon<float>())
	{
		eulerAngles.y -= _rotateAroundDelta.x * 0.002f;
		// Clamp the pitch angle to keep the camera within between -30 and 78.75 degrees.
		eulerAngles.y = glm::clamp(eulerAngles.y, -1.0f / 6.0f * glm::pi<float>(), 7.0f / 16.0f * glm::pi<float>());
		// TODO: special case if rotate_around_pressed is false
	}

	// TODO: restrict to only middle or keyboard state
	// FIXME: This only translates?
	{
		// Compute a keyboard input offset based on the current pitch and yaw.
		const auto clampedTan = glm::clamp(glm::tan(eulerAngles.y), 0.2f, 2.0f);
		const auto localMovement = _keyBoardMoveDelta * glm::vec2(1.0f / clampedTan, -1.0f) * scalingFactor * 0.001f;
		const auto planarMovement = glm::vec2(glm::cos(-eulerAngles.x), glm::sin(-eulerAngles.x));
		const auto offset = glm::vec3(localMovement.y * planarMovement.x - localMovement.x * planarMovement.y, 0.0,
		                              glm::dot(localMovement, planarMovement));
		_targetOrigin += offset;
		_targetFocus += offset;
		_focusAtClick += offset;
	}

	// TODO: zoom_delta to average island distance

	// TODO: Hand status

	// Reset deltas once they're consumed
	_keyBoardMoveDelta = glm::zero<glm::vec2>();
	_rotateAroundDelta = glm::zero<glm::vec3>();
}

void DefaultWorldCameraModel::Update(std::chrono::microseconds dt, const Camera& camera)
{
	// Get current curve interpolated values from camera
	_targetOrigin = camera.GetTargetPosition();
	_targetFocus = camera.GetTargetFocus();

	// TODO: Replace with drag focus on land
	_focusAtClick = _targetFocus;

	// Get angles (yaw, pitch, roll). Roll is always 0
	glm::vec3 eulerAngles;
	{
		const auto diff = _targetOrigin - _focusAtClick;
		// If the camera is directly above the focus point, set pitch to 90 degrees.
		if (glm::all(glm::lessThan(glm::abs(glm::xz(diff)), glm::vec2(0.1f, 0.1f))))
		{
			eulerAngles = glm::vec3(0.0f, glm::half_pi<float>(), 0.0f);
		}
		// Otherwise, calculate yaw and pitch based on the direction to the focus point.
		else
		{
			eulerAngles =
			    glm::vec3(glm::pi<float>() - glm::atan(diff.x, -diff.z), glm::atan(diff.y, glm::length(glm::xz(diff))), 0.0f);
		}
	}

	// TODO get factor
	float scalingFactor = 100.0f;
	// Adjust camera's orientation based on user input. Call will reset deltas.
	TiltZoom(eulerAngles, scalingFactor);
}

void DefaultWorldCameraModel::HandleActions(std::chrono::microseconds dt)
{
	constexpr auto SPEED = 400.0f;
	// Compute delta position (dp) based on the elapsed time and speed.
	const auto dp = SPEED * std::chrono::duration_cast<std::chrono::duration<float>>(dt).count();
	const auto& actionSystem = Locator::gameActionSystem::value();

	// TODO: Set tricons based on ZOOM or TILT if any move is detected

	if (actionSystem.GetAny(input::BindableActionMap::ROTATE_LEFT, input::BindableActionMap::ROTATE_RIGHT))
	{
		float distance = (actionSystem.Get(input::BindableActionMap::ROTATE_LEFT) ? -1.0f : 1.0f) * dp;
		_rotateAroundDelta.y += distance;
	}

	if (actionSystem.GetAny(input::BindableActionMap::TILT_UP, input::BindableActionMap::TILT_DOWN))
	{
		float distance = (actionSystem.Get(input::BindableActionMap::TILT_DOWN) ? -1.0f : 1.0f) * dp;
		_rotateAroundDelta.x += distance;
	}

	if (actionSystem.GetAny(input::BindableActionMap::MOVE_FORWARDS, input::BindableActionMap::MOVE_BACKWARDS))
	{
		float distance = (actionSystem.Get(input::BindableActionMap::MOVE_FORWARDS) ? -1.0f : 1.0f) * dp;
		// If ZOOM_ON is active, apply the movement as a zoom action.
		if (actionSystem.Get(input::BindableActionMap::ZOOM_ON))
		{
			_rotateAroundDelta.z += distance;
		}
		// If ROTATE_ON is active, apply the movement as a tilt action.
		// TODO: fight will always be rotating
		else if (actionSystem.Get(input::BindableActionMap::ROTATE_ON))
		{
			_rotateAroundDelta.x += distance;
		}
		// Otherwise, apply the movement normally.
		else
		{
			_keyBoardMoveDelta.x += distance;
		}
	}

	if (actionSystem.GetAny(input::BindableActionMap::MOVE_RIGHT, input::BindableActionMap::MOVE_LEFT))
	{
		float distance = (actionSystem.Get(input::BindableActionMap::MOVE_RIGHT) ? -1.0f : 1.0f) * dp;
		// If ZOOM_ON is active, apply the movement as a zoom action.
		if (actionSystem.Get(input::BindableActionMap::ZOOM_ON))
		{
			_rotateAroundDelta.y += distance;
		}
		// If ROTATE_ON is active, apply the movement as a tilt action.
		// TODO: fight will always be rotating
		else if (actionSystem.Get(input::BindableActionMap::ROTATE_ON))
		{
			_rotateAroundDelta.y += distance;
		}
		// Otherwise, apply the movement normally.
		else
		{
			_keyBoardMoveDelta.y -= distance;
		}
	}

	if (actionSystem.GetAny(input::BindableActionMap::ZOOM_IN, input::BindableActionMap::ZOOM_OUT))
	{
		float distance = (actionSystem.Get(input::BindableActionMap::ZOOM_IN) ? -1.0f : 1.0f) * dp;
		_rotateAroundDelta.z += distance;
	}

	// TODO: Hand-based movement
}

glm::vec3 DefaultWorldCameraModel::GetTargetPosition() const
{
	return _targetOrigin;
}

glm::vec3 DefaultWorldCameraModel::GetTargetFocus() const
{
	return _targetFocus;
}

std::chrono::seconds DefaultWorldCameraModel::GetIdleTime() const
{
	SPDLOG_LOGGER_WARN(spdlog::get("game"), "TODO: Idle Time not implemented");
	return {};
}
