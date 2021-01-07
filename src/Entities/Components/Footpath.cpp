/*****************************************************************************
 * Copyright (c) 2018-2021 openblack developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/openblack/openblack
 *
 * openblack is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#include "Footpath.h"

#include <glm/gtx/norm.hpp>

#include <Entities/Registry.h>
#include <Game.h>

#include "Fixed.h"
#include "Transform.h"

using namespace openblack;
using namespace openblack::entities::components;

std::optional<uint32_t> Footpath::GetNextNode(uint32_t nodeIndex, bool backwards) const
{
	for (int i = static_cast<int>(nodeIndex) + (backwards ? -1 : 1);
	     backwards ? (i > -1) : (i < static_cast<int>(nodes.size())); //
	     i = i + (backwards ? -1 : 1))
	{
		// TODO(bwrsandman): check the fields flags for 0b00001100, if either are set continue
		return i;
	}
	return std::nullopt;
}

std::optional<uint32_t> Footpath::GetNearestPos(const glm::vec3& coords, bool backwards) const
{
	auto closestNode =
	    std::min_element(nodes.cbegin(), nodes.cend(), [&coords](const Footpath::Node& lhs, const Footpath::Node& rhs) -> bool {
		    return glm::distance2(lhs.position, coords) < glm::distance2(rhs.position, coords);
	    });

	if (closestNode != nodes.cend())
	{
		auto closestNodeIndex = std::distance(nodes.cbegin(), closestNode);
		auto nextNodeIndex = GetNextNode(closestNodeIndex, backwards);
		if (nextNodeIndex)
		{
			const auto& nextNode = nodes[*nextNodeIndex];
			if (glm::distance2(nextNode.position, coords) < glm::distance2(nextNode.position, closestNode->position))
			{
				return *nextNodeIndex;
			}
		}
		return closestNodeIndex;
	}

	return std::nullopt;
}

bool FootpathLink::UseFootpathIfNecessary(Villager& living, const glm::vec3& coords, LivingState state,
                                          entt::entity destination) const
{
	const auto& registry = Game::instance()->GetEntityRegistry();

	{
		bool backwards;
		const auto nearestPathResult = GetNearestPathTo(registry.As<Transform>(living).position, coords, 40.0f, backwards);
		if (nearestPathResult.has_value())
		{
			const auto& [footpathId, nodeIndex] = nearestPathResult.value();
			if (living.SetupMoveOnFootpath(registry.Get<Footpath>(static_cast<entt::entity>(footpathId)), coords, state))
			{
				return true;
			}
		}
	}

	std::optional<Town::Id> townId;
	if (registry.Has<Abode>(destination))
	{
		townId = registry.As<Abode>(destination).townId;
	}

	if (!townId.has_value())
	{
		townId = Town::FindClosest(coords, std::numeric_limits<float>::infinity());
	}

	if (townId.has_value())
	{
		const auto& town = registry.Get<Town>(static_cast<entt::entity>(*townId));
		auto pitEntity = town.GetStoragePit();
		if (pitEntity.has_value() && &pitEntity->get() != &destination)
		{
			const auto& pit = registry.Get<MultiMapFixed>(*pitEntity);
			auto arrivePos = pit.GetArrivePos();

			bool backwards;
			const auto nearestPathResult = GetNearestPathToQuick(coords, arrivePos, 10.0f, backwards);
			if (nearestPathResult.has_value() && pit.footpathLink)
			{
				const auto& pitLink = registry.Get<FootpathLink>(static_cast<entt::entity>(*pit.footpathLink));
				auto pitNearestPathResult = pitLink.GetNearestPathToQuick(coords, arrivePos, 40.0f, backwards);
				if (pitNearestPathResult.has_value())
				{
					const auto& [FootpathId, nodeIndex] = nearestPathResult.value();
					const auto& [pitFootpathId, pitNodeIndex] = pitNearestPathResult.value();
					assert(false);
					// living.SetupMoveToOnFootpath(pitFootpath, backwards, state, pitNodeIndex);
					// living.footpath = footpath;
					// return true;
				}
			}
		}
	}
	return living.SetupMoveToWithHug(coords, state);
}

std::optional<std::pair<Footpath::Id, uint32_t>>
FootpathLink::GetNearestPathTo(const glm::vec3& current, const glm::vec3& destination, float distance, bool& outBackwards) const
{
	const auto& registry = Game::instance()->GetEntityRegistry();
	// Work with distance squared
	float bestDistance2 = distance * distance;

	std::optional<std::pair<Footpath::Id, uint32_t>> result;

	for (const auto& footpathId : footpaths)
	{
		const auto& footpath = registry.Get<Footpath>(static_cast<entt::entity>(footpathId));
		// TODO(bwrsandman):
		// if (!footpath.field_0x20) continue;
		if (!footpath.nodes.empty())
		{
			const auto& nodeHead = footpath.nodes[0];
			const auto& nodeTail = footpath.nodes[footpath.nodes.size() - 1];

			// If the tail is further away from the destination than the head, we are going backwards
			bool backwards = glm::distance2(nodeTail.position, destination) > glm::distance2(nodeHead.position, destination);

			std::optional<uint32_t> nearestNodeIndex = footpath.GetNearestPos(current, backwards);
			if (nearestNodeIndex.has_value())
			{
				const auto& nearestNode = footpath.nodes[*nearestNodeIndex];
				float testDistance2 = glm::distance2(nearestNode.position, current);
				if (testDistance2 < bestDistance2)
				{
					bestDistance2 = testDistance2;
					outBackwards = backwards;
					result = std::make_pair(footpathId, *nearestNodeIndex);
				}
			}
		}
	}
	return result;
}

std::optional<std::pair<Footpath::Id, uint32_t>> FootpathLink::GetNearestPathToQuick(const glm::vec3& current,
                                                                                     const glm::vec3& destination,
                                                                                     float distance, bool& outBackwards) const
{
	assert(false);
}
