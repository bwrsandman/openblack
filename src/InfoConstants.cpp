/*****************************************************************************
 * Copyright (c) 2018-2021 openblack developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/openblack/openblack
 *
 * openblack is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#include "InfoConstants.h"

#include <spdlog/spdlog.h>

#include "Enums.h"

using namespace openblack;
using namespace openblack::entities::components;

std::optional<std::reference_wrapper<const GAbodeInfo>> InfoConstants::GetAbodeInfo(Abode::Info id) const
{
	if (id == Abode::Info::None)
	{
		spdlog::error("Abode info is invalid");
		return std::nullopt;
	}

	if (static_cast<uint32_t>(id) >= abode.size())
	{
		spdlog::error("Abode info index ({}) is out of bounds", id);
		return std::nullopt;
	}

	return std::cref(abode[static_cast<uint32_t>(id)]);
}

std::optional<std::reference_wrapper<const GVillagerInfo>> InfoConstants::GetVillagerInfo(const Villager::Type& type) const
{
	using raw_t = std::underlying_type<Villager::Info>::type;
	auto role = std::get<Villager::Role>(type);
	Villager::Info id;
	if (Villager::IsImportantRole(role))
	{
		auto roleOffset = static_cast<raw_t>(role) - static_cast<raw_t>(Villager::Role::PiedPiper);
		id = static_cast<Villager::Info>(static_cast<raw_t>(Villager::Info::PiedPiper) + roleOffset);
	}
	else
	{
		auto rawId = static_cast<raw_t>(std::get<Tribe>(type)) * 7;
		// bwrsandman: Sexist behaviour in vanilla to have females as only housewife
		if (std::get<Villager::Sex>(type) == Villager::Sex::MALE)
		{
			switch (role)
			{
			default: // bwrsandman: assuming default male is forester
			case Villager::Role::FORESTER:
				rawId += static_cast<raw_t>(Villager::Info::CelticForesterMale);
				break;
			case Villager::Role::FISHERMAN:
				rawId += static_cast<raw_t>(Villager::Info::CelticFishermanMale);
				break;
			case Villager::Role::FARMER:
				rawId += static_cast<raw_t>(Villager::Info::CelticFarmerMale);
				break;
			case Villager::Role::SHEPHERD:
				rawId += static_cast<raw_t>(Villager::Info::CelticShepherdMale);
				break;
			case Villager::Role::LEADER:
				rawId += static_cast<raw_t>(Villager::Info::CelticLeaderMale);
				break;
			case Villager::Role::TRADER:
				rawId += static_cast<raw_t>(Villager::Info::CelticTraderMale);
				break;
			}
		}
		id = static_cast<Villager::Info>(rawId);
	}
	if (static_cast<raw_t>(id) >= villager.size())
	{
		spdlog::error("Villager info index ({}) is out of bounds", id);
		return std::nullopt;
	}

	return std::cref(villager[static_cast<raw_t>(id)]);
}

const GVillagerStateTableInfo& InfoConstants::GetVillagerStateInfo(LivingState state) const
{
	return villagerStateTable[static_cast<uint8_t>(state)];
}
