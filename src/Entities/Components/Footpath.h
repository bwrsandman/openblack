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
#include <vector>

#include <entt/fwd.hpp>
#include <glm/vec3.hpp>

#include <Enums.h>

namespace openblack::entities::components
{

struct Villager;

/// A list-like structure of positions a Living can travel on to get from the first node to the last, and vice-versa
struct Footpath
{
	using Id = int;

	struct Node
	{
		glm::vec3 position;
	};

	std::vector<Node> nodes;

	std::optional<uint32_t> GetNextNode(uint32_t nodeIndex, bool backwards) const;
	std::optional<uint32_t> GetNearestPos(const glm::vec3& coords, bool backwards) const;
};

/// Links a [Planned]MultiMapFixed entity to a list of footpaths
/// The position is used to look-up matches. If the MMF is close enough to the link, they are connected.
/// The relationship of MMF to FPL is one2one and MMF to Footpath is one2many
struct FootpathLink
{
	using Id = int;

	glm::vec3 position;
	std::vector<Footpath::Id> footpaths;

	bool UseFootpathIfNecessary(Villager& living, const glm::vec3& coords, LivingState state, entt::entity destination) const;
	std::optional<std::pair<Footpath::Id, uint32_t>> GetNearestPathTo(const glm::vec3& current, const glm::vec3& destination,
	                                                                  float distance, bool& outBackwards) const;
	std::optional<std::pair<Footpath::Id, uint32_t>>
	GetNearestPathToQuick(const glm::vec3& current, const glm::vec3& destination, float distance, bool& backwards) const;
};

} // namespace openblack::entities::components
