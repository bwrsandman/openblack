/*****************************************************************************
 * Copyright (c) 2018-2021 openblack developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/openblack/openblack
 *
 * openblack is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#include "Abode.h"

#include <unordered_map>

#include <spdlog/spdlog.h>

#include "3D/L3DMesh.h"
#include "3D/MeshPack.h"
#include "Entities/Registry.h"
#include "Game.h"
#include "Mesh.h"
#include "Town.h"
#include "Transform.h"
#include "Villager.h"

using namespace openblack::entities::components;

namespace
{
std::unordered_map<std::string, Abode::Info> abodeIdLookup {
    // Norse
    {"NORSE_ABODE_TOWN_CENTRE", Abode::Info::NorseTownCentre},
    {"NORSE_ABODE_STORAGE_PIT", Abode::Info::NorseStoragePit},
    {"NORSE_ABODE_GRAVEYARD", Abode::Info::NorseGraveyard},
    {"NORSE_ABODE_WORKSHOP", Abode::Info::NorseWorkshop},
    {"NORSE_ABODE_CRECHE", Abode::Info::NorseCreche},
    {"NORSE_ABODE_A", Abode::Info::NorseHut},
    {"NORSE_ABODE_B", Abode::Info::NorseShackX},
    {"NORSE_ABODE_C", Abode::Info::NorseShackY},
    {"NORSE_ABODE_D", Abode::Info::NorseTent},
    {"NORSE_ABODE_E", Abode::Info::NorseTempleX},
    {"NORSE_ABODE_F", Abode::Info::NorseTempleY},
    {"NORSE_ABODE_WONDER", Abode::Info::NorseWonder},
    // Celtic
    {"CELTIC_ABODE_TOWN_CENTRE", Abode::Info::CelticTownCentre},
    {"CELTIC_ABODE_STORAGE_PIT", Abode::Info::CelticStoragePit},
    {"CELTIC_ABODE_GRAVEYARD", Abode::Info::CelticGraveyard},
    {"CELTIC_ABODE_WORKSHOP", Abode::Info::CelticGraveyard},
    {"CELTIC_ABODE_CRECHE", Abode::Info::CelticCreche},
    {"CELTIC_ABODE_A", Abode::Info::CelticHut},
    {"CELTIC_ABODE_B", Abode::Info::CelticShackX},
    {"CELTIC_ABODE_C", Abode::Info::CelticShackY},
    {"CELTIC_ABODE_D", Abode::Info::CelticTent},
    {"CELTIC_ABODE_E", Abode::Info::CelticTempleX},
    {"CELTIC_ABODE_F", Abode::Info::CelticTempleY},
    {"CELTIC_ABODE_WONDER", Abode::Info::CelticWonder},
    // Japanese
    {"JAPANESE_ABODE_TOWN_CENTRE", Abode::Info::JapaneseTownCentre},
    {"JAPANESE_ABODE_STORAGE_PIT", Abode::Info::JapaneseStoragePit},
    {"JAPANESE_ABODE_GRAVEYARD", Abode::Info::JapaneseGraveyard},
    {"JAPANESE_ABODE_WORKSHOP", Abode::Info::JapaneseWorkshop},
    {"JAPANESE_ABODE_CRECHE", Abode::Info::JapaneseCreche},
    {"JAPANESE_ABODE_A", Abode::Info::JapaneseHut},
    {"JAPANESE_ABODE_B", Abode::Info::JapaneseShackX},
    {"JAPANESE_ABODE_C", Abode::Info::JapaneseShackY},
    {"JAPANESE_ABODE_D", Abode::Info::JapaneseTent},
    {"JAPANESE_ABODE_E", Abode::Info::JapaneseTempleX},
    {"JAPANESE_ABODE_F", Abode::Info::JapaneseTempleY},
    {"JAPANESE_ABODE_WONDER", Abode::Info::JapaneseWonder},
    // Aztec
    {"AZTEC_ABODE_TOWN_CENTRE", Abode::Info::AztecTownCentre},
    {"AZTEC_ABODE_STORAGE_PIT", Abode::Info::AztecStoragePit},
    {"AZTEC_ABODE_GRAVEYARD", Abode::Info::AztecGraveyard},
    {"AZTEC_ABODE_WORKSHOP", Abode::Info::AztecGraveyard},
    {"AZTEC_ABODE_CRECHE", Abode::Info::AztecCreche},
    {"AZTEC_ABODE_A", Abode::Info::AztecHut},
    {"AZTEC_ABODE_B", Abode::Info::AztecShackX},
    {"AZTEC_ABODE_C", Abode::Info::AztecShackY},
    {"AZTEC_ABODE_D", Abode::Info::AztecTent},
    {"AZTEC_ABODE_E", Abode::Info::AztecTempleX},
    {"AZTEC_ABODE_F", Abode::Info::AztecTempleY},
    {"AZTEC_ABODE_WONDER", Abode::Info::AztecWonder},
    // Tibetan
    {"TIBETAN_ABODE_TOWN_CENTRE", Abode::Info::TibetanTownCentre},
    {"TIBETAN_ABODE_STORAGE_PIT", Abode::Info::TibetanStoragePit},
    {"TIBETAN_ABODE_GRAVEYARD", Abode::Info::TibetanGraveyard},
    {"TIBETAN_ABODE_WORKSHOP", Abode::Info::TibetanWorkshop},
    {"TIBETAN_ABODE_CRECHE", Abode::Info::TibetanCreche},
    {"TIBETAN_ABODE_A", Abode::Info::TibetanHut},
    {"TIBETAN_ABODE_B", Abode::Info::TibetanShackX},
    {"TIBETAN_ABODE_C", Abode::Info::TibetanShackY},
    {"TIBETAN_ABODE_D", Abode::Info::TibetanTent},
    {"TIBETAN_ABODE_E", Abode::Info::TibetanTempleX},
    {"TIBETAN_ABODE_F", Abode::Info::TibetanTempleY},
    {"TIBETAN_ABODE_WONDER", Abode::Info::TibetanWonder},
    // American Indian
    {"INDIAN_ABODE_TOWN_CENTRE", Abode::Info::IndianTownCentre},
    {"INDIAN_ABODE_STORAGE_PIT", Abode::Info::IndianStoragePit},
    {"INDIAN_ABODE_GRAVEYARD", Abode::Info::IndianGraveyard},
    {"INDIAN_ABODE_WORKSHOP", Abode::Info::IndianWorkshop},
    {"INDIAN_ABODE_CRECHE", Abode::Info::IndianCreche},
    {"INDIAN_ABODE_A", Abode::Info::IndianHut},
    {"INDIAN_ABODE_B", Abode::Info::IndianShackX},
    {"INDIAN_ABODE_C", Abode::Info::IndianShackY},
    {"INDIAN_ABODE_D", Abode::Info::IndianTent},
    {"INDIAN_ABODE_E", Abode::Info::IndianTempleX},
    {"INDIAN_ABODE_F", Abode::Info::IndianTempleY},
    {"INDIAN_ABODE_WONDER", Abode::Info::IndianWonder},
    // Greek
    {"GREEK_ABODE_TOWN_CENTRE", Abode::Info::GreekTownCentre},
    {"GREEK_ABODE_STORAGE_PIT", Abode::Info::GreekStoragePit},
    {"GREEK_ABODE_GRAVEYARD", Abode::Info::GreekGraveyard},
    {"GREEK_ABODE_WORKSHOP", Abode::Info::GreekWorkshop},
    {"GREEK_ABODE_CRECHE", Abode::Info::GreekCreche},
    {"GREEK_ABODE_A", Abode::Info::GreekHut},
    {"GREEK_ABODE_B", Abode::Info::GreekShackX},
    {"GREEK_ABODE_C", Abode::Info::GreekShackY},
    {"GREEK_ABODE_D", Abode::Info::GreekTent},
    {"GREEK_ABODE_E", Abode::Info::GreekTempleX},
    {"GREEK_ABODE_F", Abode::Info::GreekTempleY},
    {"GREEK_ABODE_WONDER", Abode::Info::GreekWonder},
};
}

std::optional<Abode::Info> Abode::GetInfo(const std::string& abodeType)
{
	auto item = abodeIdLookup.find(abodeType);

	if (item == abodeIdLookup.end())
	{
		SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "Missing abode mesh lookup for \"{}\".", abodeType);
		return std::nullopt;
	}

	return item->second;
}

void Abode::Create(uint32_t townId, const glm::vec3& position, const std::string& abodeInfo, const glm::mat4& rotation,
                   const glm::vec3& size, uint32_t foodAmount, uint32_t woodAmount, bool planned)
{
	auto& registry = Game::instance()->GetEntityRegistry();

	std::optional<Town::Id> town;

	// If there is no town, assign to closest
	const auto townItr = registry.Context().towns.find(townId);
	if (townItr != registry.Context().towns.end())
	{
		town = townItr->first;
	}
	else
	{
		town = Town::FindClosest(position, std::numeric_limits<float>::infinity());
	}

	const auto info = GetInfo(abodeInfo);

	if (town && info)
	{
		if (planned)
		{
			SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "Function {} not implemented with planned=true.", __func__);
		}
		else
		{
			const auto entity = registry.Create();

			registry.Assign<Transform>(entity, position, rotation, size);
			registry.Assign<Fixed>(entity);
			registry.Assign<MultiMapFixed>(entity, std::nullopt);
			const auto& abode =
			    registry.Assign<Abode>(entity, info.value(), static_cast<int>(town.value()), foodAmount, woodAmount);
			registry.Assign<Mesh>(entity, abodeMeshLookup[abode.type], static_cast<int8_t>(0), static_cast<int8_t>(0));
		}
	}
	else if (!town.has_value())
	{
		SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "Function {} has invalid Town ({}).", __func__, townId);
	}
	else if (!info.has_value())
	{
		SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "Function {} has invalid Abode Info ({}).", __func__, abodeInfo);
	}
}

void Abode::AddVillager(entt::entity entity)
{
	const auto& infoConstants = Game::instance()->GetInfoConstants();
	auto& registry = Game::instance()->GetEntityRegistry();
	auto& registryContext = registry.Context();

	const auto info = infoConstants.GetAbodeInfo(type);
	if (info.has_value())
	{
		auto& villager = registry.Get<Villager>(entity);
		// TODO(bwrsandman): if already assigned to abode or villager homeless list, remove
		assert(!villager.abode.has_value());
		assert(villager.town.value_or(registryContext.towns[townId]) == registryContext.towns[townId]);
		if (static_cast<int>(inhabitants.size()) < info->get().maxCapacity)
		{
			inhabitants.insert(entity);
			villager.abode = registry.ToEntity(*this);
		}
		else
		{
			SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "Attempting to add villager to abode beyond capacity");
			assert(false);
		}
	}
}

glm::vec3 Abode::GetDoorOffset() const
{
	const auto meshId = abodeMeshLookup[type];
	const auto& mesh = Game::instance()->GetMeshPack().GetMesh(meshId);
	auto doorPos = mesh.GetDoorPos();
	return doorPos.value_or(glm::vec3());
}

bool Abode::IsFunctional() const
{
	// TODO(bwrsandman)
	return true;
}
