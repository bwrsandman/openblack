/*****************************************************************************
 * Copyright (c) 2018-2021 openblack developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/openblack/openblack
 *
 * openblack is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <Enums.h>

namespace openblack::entities::components
{

struct Villager;

struct Transform
{
	glm::vec3 position;
	glm::mat3 rotation;
	glm::vec3 scale;

	explicit operator glm::mat4() const;
	// TODO(bwrsandman): Change to Living component after they are added
	bool UseFootpathIfNecessary(Villager& living, const glm::vec3& coords, LivingState state) const;
};

} // namespace openblack::entities::components
