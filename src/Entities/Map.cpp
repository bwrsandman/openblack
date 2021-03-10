/*****************************************************************************
 * Copyright (c) 2018-2021 openblack developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/openblack/openblack
 *
 * openblack is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#include "Map.h"

#include <glm/gtx/vec_swizzle.hpp>
#include <glm/vec3.hpp>

#include <Entities/Components/Fixed.h>
#include <Entities/Components/Transform.h>
#include <Entities/Registry.h>
#include <Game.h>

glm::u16vec2 openblack::entities::Map::GetGridCell(const glm::vec3& pos)
{
	const glm::u32vec2 coords = glm::xz(pos) * static_cast<float>(0x1000) / 10.0f;
	const glm::u16vec2 result(coords.x >> 0x10, coords.y >> 0x10);
	assert(glm::all(glm::lessThan(result, _gridSize)));
	return result;
}

void openblack::entities::Map::Clear()
{
	for (auto& g : _fixedGrid)
	{
		g.clear();
	}
	for (auto& g : _mobileGrid)
	{
		g.clear();
	}
}

const std::unordered_set<entt::entity>& openblack::entities::Map::GetFixedInGridCell(const glm::vec3& pos) const
{
	const auto cellId = GetGridCell(pos);
	return _fixedGrid[cellId.x + cellId.y * _gridSize.x];
}

const std::unordered_set<entt::entity>& openblack::entities::Map::GetMobileInGridCell(const glm::vec3& pos) const
{
	const auto cellId = GetGridCell(pos);
	return _mobileGrid[cellId.x + cellId.y * _gridSize.x];
}

void openblack::entities::Map::Rebuild()
{
	using namespace openblack::entities::components;

	auto& registry = Game::instance()->GetEntityRegistry();
	Clear();
	registry.Each<const Fixed, const Transform>(
	    [this](entt::entity entity, const Fixed& fixed, const Transform& transform) { AddFixed(transform.position, entity); });
	registry.Each<const Mobile, const Transform>([this](entt::entity entity, const Mobile& mobile, const Transform& transform) {
		AddMobile(transform.position, entity);
	});
}

void openblack::entities::Map::AddFixed(const glm::vec3& pos, entt::entity fixed)
{
	const auto cellId = GetGridCell(pos);
	auto& cell = _fixedGrid[cellId.x + cellId.y * _gridSize.x];
	cell.insert(fixed);
}

void openblack::entities::Map::AddMobile(const glm::vec3& pos, entt::entity mobile)
{
	const auto cellId = GetGridCell(pos);
	auto& cell = _mobileGrid[cellId.x + cellId.y * _gridSize.x];
	cell.insert(mobile);
}
