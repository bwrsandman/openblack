/*****************************************************************************
 * Copyright (c) 2018-2021 openblack developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/openblack/openblack
 *
 * openblack is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#include "Town.h"

#include <glm/vec3.hpp>

#include "Entities/Registry.h"
#include "Game.h"
#include "Transform.h"

using namespace openblack::entities::components;

std::optional<Town::Id> Town::FindClosest(const glm::vec3& point)
{
	const auto& registry = Game::instance()->GetEntityRegistry();

	std::optional<Town::Id> result = std::nullopt;
	auto closest = std::numeric_limits<float>::infinity();

	registry.Each<const Town, const Transform>([&point, &result, &closest](auto town, auto transform) {
		float distance_2 = glm::dot(point, transform.position);
		if (distance_2 < closest)
		{
			closest = distance_2;
			result = town.id;
		}
	});

	return result;
}
