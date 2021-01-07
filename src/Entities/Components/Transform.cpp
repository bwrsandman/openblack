/*****************************************************************************
 * Copyright (c) 2018-2021 openblack developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/openblack/openblack
 *
 * openblack is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#include "Transform.h"

#include <Entities/Registry.h>
#include <Game.h>

#include "Fixed.h"
#include "Footpath.h"
#include "Villager.h"

using namespace openblack;
using namespace openblack::entities::components;

Transform::operator glm::mat4() const
{
	glm::mat4 modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::translate(modelMatrix, position);
	modelMatrix *= glm::mat4(rotation);
	modelMatrix = glm::scale(modelMatrix, scale);
	return modelMatrix;
}

bool Transform::UseFootpathIfNecessary(Villager& living, const glm::vec3& coords, LivingState state) const
{
	const auto& registry = Game::instance()->GetEntityRegistry();
	const auto& entity = registry.ToEntity(*this);
	if (registry.Has<MultiMapFixed>(entity))
	{
		const auto& fixed = registry.Get<MultiMapFixed>(entity);
		if (fixed.footpathLink.has_value())
		{
			const auto& link = registry.Get<FootpathLink>(static_cast<entt::entity>(*fixed.footpathLink));
			return link.UseFootpathIfNecessary(living, coords, state, entity);
		}
	}
	return living.SetupMoveToWithHug(coords, state);
}
