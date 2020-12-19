/*****************************************************************************
 * Copyright (c) 2018-2021 openblack developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/openblack/openblack
 *
 * openblack is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#include "Villager.h"

#include <stdexcept>
#include <string_view>

#include <glm/gtx/euler_angles.hpp>

#include "Common/MeshLookup.h"
#include "Entities/Registry.h"
#include "Game.h"
#include "Mesh.h"
#include "Transform.h"

using namespace openblack;
using openblack::entities::components::Villager;

void Villager::Create(const glm::vec3& abodePosition, const glm::vec3& position, Tribe tribe, Role role, uint32_t age)
{
	auto& registry = Game::instance()->GetEntityRegistry();

	const auto entity = registry.Create();
	registry.Assign<Transform>(entity, position, glm::eulerAngleY(glm::radians(180.0f)), glm::vec3(1.0));
	const uint32_t health = 100;
	const uint32_t hunger = 100;
	const auto lifeStage = age >= 18 ? Villager::LifeStage::Adult : Villager::LifeStage::Child;
	const auto sex = (role == Villager::Role::HOUSEWIFE) ? Villager::Sex::FEMALE : Villager::Sex::MALE;
	const auto task = Villager::Task::IDLE;
	const auto& villager = registry.Assign<Villager>(entity, health, age, hunger, lifeStage, sex, tribe, role, task);
	registry.Assign<Mesh>(entity, villagerMeshLookup[villager.GetVillagerType()], static_cast<int8_t>(0),
	                      static_cast<int8_t>(0));
}

bool Villager::IsImportantRole(Villager::Role role)
{
	switch (role)
	{
	case Role::PiedPiper:
	case Role::Shaolin:
	case Role::IdolBuilder:
	case Role::Hermit:
	case Role::Hippy:
	case Role::Priest:
	case Role::Priestess:
	case Role::Marauder:
	case Role::Footballer_1:
	case Role::Footballer_2:
	case Role::Engineer:
	case Role::Shepered:
	case Role::Nomade:
	case Role::AztecLeader:
	case Role::CreatureTrainer:
	case Role::NorseSailor:
	case Role::Breeder:
	case Role::Healer:
	case Role::Sculptor:
	case Role::Crusader:
	case Role::SailorAccordian:
		return true;
	default:
		return false;
	}
}

Villager::Type Villager::GetVillagerType() const
{
	auto importantRole = role;

	if (!Villager::IsImportantRole(role))
	{
		importantRole = Role::NONE;
	}

	Villager::Type villagerType = {tribe, lifeStage, sex, importantRole};

	return villagerType;
}
