/*****************************************************************************
 * Copyright (c) 2018-2023 openblack developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/openblack/openblack
 *
 * openblack is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#pragma once

#include <glm/vec2.hpp>

#include "CameraModel.h"

namespace openblack
{
class DefaultWorldCameraModel final: public CameraModel
{
	enum class Mode
	{
		Cartesian,
		Polar,
	};

public:
	DefaultWorldCameraModel();
	~DefaultWorldCameraModel() final;

	void Update(std::chrono::microseconds dt, const Camera& camera) final;
	void HandleActions(std::chrono::microseconds dt) final;
	glm::vec3 GetTargetPosition() const final;
	glm::vec3 GetTargetFocus() const final;
	std::chrono::seconds GetIdleTime() const final;

private:
	/// Modifies the given Euler angles based on the rotate Around and keyboard Move Deltas for rotation and zoom.
	/// @param eulerAngles A reference representing Euler angles (yaw, pitch, roll) to be adjusted. Roll is always 0.
	void TiltZoom(glm::vec3& eulerAngles, float scalingFactor);
	/// Computes the harmonic mean of the distances from a point of origin to a set of points determined by raycasting in screen
	/// space.
	///
	/// The function casts 16 rays from the center of the screen to vertically distributed points on the screen.
	/// The harmonic mean of these distances is then calculated by averaging their reciprocals and taking the reciprocal of that
	/// average.
	///
	/// @return The harmonic mean of the distances from the origin to each hit point.
	float GetVerticalLineInverseDistanceWeighingRayCast(const Camera& camera) const;
	/// Corrects altitude of the camera
	/// @return If a modification to the camera position was applied.
	bool ConstrainAltitude();
	/// Corrects distance of the camera from the island
	/// @return If a modification to the camera position was applied.
	bool ConstrainDisc();

	Mode _mode = Mode::Cartesian;

	// Values from target camera state which the camera may interpolate to. Not the current camera state.
	glm::vec3 _targetOrigin;
	glm::vec3 _targetFocus;

	// State of input Action
	glm::vec3 _rotateAroundDelta = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec2 _keyBoardMoveDelta = glm::vec2(0.0f, 0.0f);

	// Estimate of camera to island geometry
	float _averageIslandDistance = 0.0f;

	// Updated at the start of a click+drag
	float _distanceAtClick = 0.0f;
	glm::vec3 _focusAtClick = glm::vec3(0.0f, 0.0f, 0.0f);
};
} // namespace openblack
