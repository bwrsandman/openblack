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
#include <set>
#include <string>
#include <unordered_map>

#include <entt/fwd.hpp>
#include <glm/vec3.hpp>

#include "Enums.h"

namespace openblack::entities::components
{

struct Town
{
	using Id = int;

	Id id;
	Tribe tribe;
	std::unordered_map<std::string, float> beliefs;
	bool uninhabitable = false;

	glm::vec3 cachedCongregationPosition;
	std::set<entt::entity> homelessVillagers;

	static std::optional<Town::Id> FindClosest(const glm::vec3& point, float maxDistance);

	std::optional<entt::entity> FindAbodeWithSpace() const;
	void AddHomelessVillager(entt::entity entity);
	const glm::vec3& GetCongregationPos();
	std::optional<std::reference_wrapper<const entt::entity>> GetStoragePit() const;
};

} // namespace openblack::entities::components
