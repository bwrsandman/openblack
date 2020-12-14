/*****************************************************************************
 * Copyright (c) 2018-2021 openblack developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/openblack/openblack
 *
 * openblack is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#pragma once

#include <optional>
#include <string>
#include <unordered_map>

#include <glm/fwd.hpp>

namespace openblack::entities::components
{

struct Town
{
	using Id = int;

	Id id;
	std::unordered_map<std::string, float> beliefs;
	bool uninhabitable = false;

	static std::optional<Town::Id> FindClosest(const glm::vec3& point);
};

} // namespace openblack::entities::components
