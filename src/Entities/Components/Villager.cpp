/*****************************************************************************
 * Copyright (c) 2018-2021 openblack developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/openblack/openblack
 *
 * openblack is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#include "Villager.h"

#include <Common/MeshLookup.h>
#include <Entities/Components/Footpath.h>

using namespace openblack;
using openblack::entities::components::Villager;

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

bool Villager::SetupMoveOnFootpath(const Footpath& footpath, const glm::vec3& coords)
{
	// TODO(bwrsandman):
	//   get current node from *this
	//   remove *this from current node's follower list
	//   update current node based on Transform(this)->position
	//   add *this to new current node
	//   call SetupMobileMoveToPos
	//   call virtual function 0x8dc and if returns true, call 0x8fc and return true
	assert(false);
}
