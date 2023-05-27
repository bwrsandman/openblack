/*****************************************************************************
 * Copyright (c) 2018-2023 openblack developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/openblack/openblack
 *
 * openblack is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#include "DefaultWorldCameraModel.h"

#include <spdlog/spdlog.h>

#include "Camera.h"

using namespace openblack;

DefaultWorldCameraModel::DefaultWorldCameraModel() = default;

DefaultWorldCameraModel::~DefaultWorldCameraModel() = default;

void DefaultWorldCameraModel::Update(std::chrono::microseconds dt, const Camera& camera)
{
	// Get current curve interpolated values from camera
	_targetOrigin = camera.GetTargetPosition();
	_targetFocus = camera.GetTargetFocus();
}

void DefaultWorldCameraModel::HandleActions(std::chrono::microseconds dt) {}

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
