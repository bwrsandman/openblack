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

using namespace openblack;

DefaultWorldCameraModel::DefaultWorldCameraModel() = default;

DefaultWorldCameraModel::~DefaultWorldCameraModel() = default;

void DefaultWorldCameraModel::Update(std::chrono::microseconds dt, const Camera& camera) {}

void DefaultWorldCameraModel::HandleActions(std::chrono::microseconds dt, const Camera& camera) {}

glm::vec3 DefaultWorldCameraModel::GetTargetPosition() const
{
	return {0.0f, 10.0f, 0.0f};
}

glm::vec3 DefaultWorldCameraModel::GetTargetFocus() const
{
	return {0.0f, 10.0f, 1.0f};
}

std::chrono::seconds DefaultWorldCameraModel::GetIdleTime() const
{
	SPDLOG_LOGGER_WARN(spdlog::get("game"), "TODO: Idle Time not implemented");
	return {};
}
