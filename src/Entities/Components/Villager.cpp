/*****************************************************************************
 * Copyright (c) 2018-2021 openblack developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/openblack/openblack
 *
 * openblack is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#include "Villager.h"

#include <cassert>
#include <stdexcept>
#include <string_view>

#include <glm/gtx/euler_angles.hpp>

#include "Abode.h"
#include "Common/MeshLookup.h"
#include "Entities/Registry.h"
#include "Game.h"
#include "Mesh.h"
#include "Town.h"
#include "Transform.h"

using namespace openblack;
using namespace openblack::entities::components;

void Villager::Create(const glm::vec3& abodePosition, const glm::vec3& position, Tribe tribe, Role role, uint32_t age)
{
	const auto& infoConstants = Game::instance()->GetInfoConstants();
	auto& registry = Game::instance()->GetEntityRegistry();
	auto& context = registry.Context();

	// Find Abode and Town
	std::optional<entt::entity> foundAbode;
	std::optional<Town::Id> town = std::nullopt;
	registry.Each<const Abode, const Transform>(
	    [&foundAbode, &town, &context, &infoConstants, &abodePosition](entt::entity entity, auto component, auto transform) {
		    if (foundAbode.has_value())
		    {
			    return;
		    }
		    const auto door = static_cast<glm::mat4>(transform) * glm::vec4(component.GetDoorOffset(), 1.0f);
		    if (glm::floor(abodePosition.x / 10.0f) == glm::floor(door.x / 10.0f) &&
		        glm::floor(abodePosition.z / 10.0f) == glm::floor(door.z / 10.0f))
		    {
			    auto townItr = context.towns.find(component.townId);
			    if (townItr != context.towns.end())
			    {
				    town = townItr->first;
			    }

			    const auto info = infoConstants.GetAbodeInfo(component.type);
			    if (info.has_value())
			    {
				    if (static_cast<int>(component.inhabitants.size()) < info->get().maxCapacity)
				    {
					    foundAbode = entity;
				    }
			    }
		    }
	    });

	const auto entity = registry.Create();
	registry.Assign<Transform>(entity, position, glm::eulerAngleY(glm::radians(180.0f)), glm::vec3(1.0));
	registry.Assign<Mobile>(entity);
	const uint32_t health = 100;
	const uint32_t hunger = 100;
	const auto lifeStage = age >= 18 ? Villager::LifeStage::Adult : Villager::LifeStage::Child;
	const auto sex = (role == Villager::Role::HOUSEWIFE) ? Villager::Sex::FEMALE : Villager::Sex::MALE;
	const auto task = Villager::Task::IDLE;
	const auto action = LivingAction {{LivingState::CREATED}, 0};
	const auto& villager =
	    registry.Assign<Villager>(entity, health, age, hunger, lifeStage, sex, tribe, role, task, std::move(action));
	registry.Assign<Mesh>(entity, villagerMeshLookup[villager.GetVillagerType()], static_cast<int8_t>(0),
	                      static_cast<int8_t>(0));

	if (!foundAbode.has_value() && town.has_value())
	{
		foundAbode = registry.Get<Town>(context.towns[*town]).FindAbodeWithSpace();
	}

	if (foundAbode.has_value())
	{
		registry.Get<Abode>(*foundAbode).AddVillager(entity);
	}
	else if (town.has_value())
	{
		registry.Get<Town>(context.towns[*town]).AddHomelessVillager(entity);
	}
	else
	{
		// TODO(bwrsandman): Add to a list of villagers without towns
		SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "Adding villagers without towns is not yet implemented.");
	}
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

LivingState Villager::GetState(LivingAction::Index index) const
{
	assert(index < LivingAction::Index::_Count);
	return livingAction.states[static_cast<size_t>(index)];
}

void Villager::SetState(LivingAction::Index index, LivingState state)
{
	assert(index < LivingAction::Index::_Count);
	if (livingAction.states[static_cast<size_t>(index)] != state)
	{
		livingAction.states[static_cast<size_t>(index)] = state;
		if (index == LivingAction::Index::Top)
		{
			livingAction.turnsSinceStateChange = 0;
		}
	}
}

std::optional<std::reference_wrapper<const Abode>> Villager::GetAbode() const
{
	if (!abode.has_value())
	{
		return std::nullopt;
	}

	const auto& registry = Game::instance()->GetEntityRegistry();
	auto& component = registry.Get<Abode>(abode.value());
	return std::reference_wrapper<const Abode>(component);
}

std::optional<std::reference_wrapper<Town>> Villager::GetTown() const
{
	if (!town.has_value())
	{
		return std::nullopt;
	}

	auto& registry = Game::instance()->GetEntityRegistry();
	auto& component = registry.Get<Town>(town.value());
	return std::reference_wrapper<Town>(component);
}
