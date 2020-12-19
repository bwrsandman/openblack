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

#include "Abode.h"
#include "Entities/Registry.h"
#include "Game.h"
#include "Transform.h"

using namespace openblack::entities::components;

std::optional<entt::entity> Town::FindAbodeWithSpace() const
{
	const auto& infoConstants = Game::instance()->GetInfoConstants();
	auto& registry = Game::instance()->GetEntityRegistry();

	std::optional<entt::entity> result;
	registry.Each<const Abode>([this, &infoConstants, &result](entt::entity entity, auto component) {
		if (result.has_value() || component.townId != id)
		{
			return;
		}

		const auto info = infoConstants.GetAbodeInfo(component.type);
		if (info.has_value())
		{
			if (static_cast<int>(component.inhabitants.size()) < info->get().maxCapacity)
			{
				result = entity;
			}
		}
	});

	return result;
}

void Town::AddHomelessVillager(entt::entity entity)
{
	const auto& infoConstants = Game::instance()->GetInfoConstants();
	auto& registry = Game::instance()->GetEntityRegistry();
	auto& registryContext = registry.Context();

	auto& villager = registry.Get<Villager>(entity);
	// TODO(bwrsandman): if already assigned to abode or other villager homeless list, remove
	assert(!villager.abode.has_value());
	assert(villager.town.value_or(registryContext.towns[id]) == registryContext.towns[id]);
	homelessVillagers.insert(entity);
	villager.town = std::make_optional(registry.ToEntity(*this));
}

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
