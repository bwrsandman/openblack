/*****************************************************************************
 * Copyright (c) 2018-2023 openblack developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/openblack/openblack
 *
 * openblack is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#pragma once

#include "CameraModel.h"

namespace openblack
{
class DefaultWorldCameraModel final: public CameraModel
{
public:
	DefaultWorldCameraModel();
	~DefaultWorldCameraModel() final;

	void Update(std::chrono::microseconds dt, const Camera& camera) final;
	void HandleActions(std::chrono::microseconds dt) final;
	glm::vec3 GetTargetPosition() const final;
	glm::vec3 GetTargetFocus() const final;
	std::chrono::seconds GetIdleTime() const final;

private:
	// Values from target camera state which the camera may interpolate to. Not the current camera state.
	glm::vec3 _targetOrigin;
	glm::vec3 _targetFocus;
};
} // namespace openblack
