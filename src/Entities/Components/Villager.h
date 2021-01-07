/*****************************************************************************
 * Copyright (c) 2018-2021 openblack developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/openblack/openblack
 *
 * openblack is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>

#include <entt/fwd.hpp>
#include <glm/vec3.hpp>

#include "Enums.h"

namespace openblack::entities::components
{

struct Abode;
struct Footpath;
struct Town;

struct Villager
{
	/// Originally VillagerInfo
	enum class Info : uint8_t
	{
		CelticHousewifeFemale = 0,
		CelticForesterMale = 1,
		CelticFishermanMale = 2,
		CelticFarmerMale = 3,
		CelticShepherdMale = 4,
		CelticLeaderMale = 5,
		CelticTraderMale = 6,
		AfricanHousewifeFemale = 7,
		AfricanForesterMale = 8,
		AfricanFishermanMale = 9,
		AfricanFarmerMale = 10,
		AfricanShepherdMale = 11,
		AfricanLeaderMale = 12,
		AfricanTraderMale = 13,
		AztecHousewifeFemale = 14,
		AztecForesterMale = 15,
		AztecFishermanMale = 16,
		AztecFarmerMale = 17,
		AztecShepherdMale = 18,
		AztecLeaderMale = 19,
		AztecTraderMale = 20,
		JapaneseHousewifeFemale = 21,
		JapaneseForesterMale = 22,
		JapaneseFishermanMale = 23,
		JapaneseFarmerMale = 24,
		JapaneseShepherdMale = 25,
		JapaneseLeaderMale = 26,
		JapaneseTraderMale = 27,
		IndianHousewifeFemale = 28,
		IndianForesterMale = 29,
		IndianFishermanMale = 30,
		IndianFarmerMale = 31,
		IndianShepherdMale = 32,
		IndianLeaderMale = 33,
		IndianTraderMale = 34,
		EgyptianHousewifeFemale = 35,
		EgyptianForesterMale = 36,
		EgyptianFishermanMale = 37,
		EgyptianFarmerMale = 38,
		EgyptianShepherdMale = 39,
		EgyptianLeaderMale = 40,
		EgyptianTraderMale = 41,
		GreekHousewifeFemale = 42,
		GreekForesterMale = 43,
		GreekFishermanMale = 44,
		GreekFarmerMale = 45,
		GreekShepherdMale = 46,
		GreekLeaderMale = 47,
		GreekTraderMale = 48,
		NorseHousewifeFemale = 49,
		NorseForesterMale = 50,
		NorseFishermanMale = 51,
		NorseFarmerMale = 52,
		NorseShepherdMale = 53,
		NorseLeaderMale = 54,
		NorseTraderMale = 55,
		TibetanHousewifeFemale = 56,
		TibetanForesterMale = 57,
		TibetanFishermanMale = 58,
		TibetanFarmerMale = 59,
		TibetanShepherdMale = 60,
		TibetanLeaderMale = 61,
		TibetanTraderMale = 62,
		PiedPiper = 63,
		Shaolin = 64,
		IdolBuilder = 65,
		Hermit = 66,
		Hippy = 67,
		Priest = 68,
		Priestess = 69,
		Marauder = 70,
		Footballer_1 = 71,
		Footballer_2 = 72,
		Engineer = 73,
		Shepered = 74,
		Nomade = 75,
		AztecLeader = 76,
		CreatureTrainer = 77,
		NorseSailor = 78,
		Breeder = 79,
		Healer = 80,
		Sculptor = 81,
		Crusader = 82,
		SailorAccordian = 83,
		// MuleHead                = 84, // CREATUREISLE
		// EgyptianBigEyes         = 85, // CREATUREISLE
		// Giant                   = 86, // CREATUREISLE

		_COUNT
	};

	/// Originally VillagerRoles
	enum class Role : uint8_t
	{
		NONE,
		HOUSEWIFE,
		FARMER,
		FISHERMAN,
		FORESTER,
		BREEDER,
		SHEPHERD,
		MISSIONARY,
		LEADER, // No idea what a leader is but they are spawned in land 2 and so on
		TRADER, // Exists on one of the MP land scripts
		PiedPiper,
		Shaolin,
		IdolBuilder,
		Hermit,
		Hippy,
		Priest,
		Priestess,
		Marauder,
		Footballer_1,
		Footballer_2,
		Engineer,
		Shepered,
		Nomade,
		AztecLeader,
		CreatureTrainer,
		NorseSailor,
		Breeder,
		Healer,
		Sculptor,
		Crusader,
		SailorAccordian,

		_COUNT
	};
	static constexpr std::array<std::string_view, static_cast<uint8_t>(Role::_COUNT)> RoleStrs = {
	    "NONE",            //
	    "HOUSEWIFE",       //
	    "FARMER",          //
	    "FISHERMAN",       //
	    "FORESTER",        //
	    "BREEDER",         //
	    "SHEPHERD",        //
	    "MISSIONARY",      //
	    "LEADER",          //
	    "TRADER",          //
	    "PiedPiper",       //
	    "Shaolin",         //
	    "IdolBuilder",     //
	    "Hermit",          //
	    "Hippy",           //
	    "Priest",          //
	    "Priestess",       //
	    "Marauder",        //
	    "Footballer_1",    //
	    "Footballer_2",    //
	    "Engineer",        //
	    "Shepered",        //
	    "Nomade",          //
	    "AztecLeader",     //
	    "CreatureTrainer", //
	    "NorseSailor",     //
	    "Breeder",         //
	    "Healer",          //
	    "Sculptor",        //
	    "Crusader",        //
	    "SailorAccordian"  //
	};

	/// Originally VillagerTasks
	enum class Task : uint8_t
	{
		IDLE,

		_COUNT
	};
	static constexpr std::array<std::string_view, static_cast<uint8_t>(Task::_COUNT)> TaskStrs = {
	    "IDLE", //
	};

	/// Originally VillagerSex
	enum class Sex : uint8_t
	{
		MALE,
		FEMALE,

		_COUNT
	};
	static constexpr std::array<std::string_view, static_cast<uint8_t>(Sex::_COUNT)> SexStrs = {
	    "MALE",   //
	    "FEMALE", //
	};

	/// Originally VillagerLifeStage
	enum class LifeStage : uint8_t
	{
		Child,
		Adult,

		_COUNT
	};
	static constexpr std::array<std::string_view, static_cast<uint8_t>(LifeStage::_COUNT)> LifeStageStrs = {
	    "Child", //
	    "Adult", //
	};

	using Type = std::tuple<Tribe, Villager::LifeStage, Villager::Sex, Villager::Role>;

	struct TypeId
	{
		std::size_t operator()(const Villager::Type& type) const
		{
			const auto tribe = std::get<Tribe>(type);
			const auto villagerSex = std::get<Sex>(type);
			const auto lifeStage = std::get<LifeStage>(type);
			auto role = std::get<Role>(type);
			auto h1 = std::hash<decltype(tribe)>()(tribe);
			auto h2 = std::hash<decltype(villagerSex)>()(villagerSex);
			auto h3 = std::hash<decltype(lifeStage)>()(lifeStage);

			if (!Villager::IsImportantRole(role))
			{
				role = Role::NONE;
			}

			auto h4 = std::hash<decltype(role)>()(role);
			return h1 ^ h2 ^ h3 ^ h4;
		}
	};

	struct LivingAction
	{
		enum class Index : uint8_t
		{
			Top,
			Final,
			Previous,

			_Count,
		};
		static constexpr std::array<std::string_view, static_cast<size_t>(Index::_Count)> IndexStrings = {
		    "Top",
		    "Final",
		    "Previous",
		};

		std::array<LivingState, static_cast<size_t>(Index::_Count)> states;
		uint16_t turnsUntilStateChange;
		uint16_t turnsSinceStateChange;
	};

	uint32_t health;
	uint32_t age;
	uint32_t hunger;
	LifeStage lifeStage;
	Sex sex;
	Tribe tribe;
	Role role;
	Task task;
	LivingAction livingAction;
	std::optional<entt::entity> town;
	std::optional<entt::entity> abode;

	static void Create(const glm::vec3& abodePosition, const glm::vec3& position, Tribe tribe, Role role, uint32_t age);
	static bool IsImportantRole(Role role);
	Type GetVillagerType() const;
	LivingState GetState(LivingAction::Index index) const;
	void SetState(LivingAction::Index index, LivingState state, bool skipTransition);
	std::optional<std::reference_wrapper<const Abode>> GetAbode() const;
	std::optional<std::reference_wrapper<Town>> GetTown() const;
	std::optional<glm::vec3> GetChillOutPos();
	bool GetTentPos(glm::vec3& position);
	bool SetupMoveToWithHug(const glm::vec3& coords, LivingState destinationState);
	bool SetupMoveToOnFootpath(const Abode& destination, const glm::vec3& coords, LivingState state);
	bool SetupMoveOnFootpath(const Footpath& footpath, const glm::vec3& coords, LivingState state);
	uint32_t DoGoingHome(LivingState state, LivingState homelessState);
	void LeaveHome();

	uint32_t CallState(LivingAction::Index index);
	bool CallEntryState(LivingAction::Index index, LivingState src, LivingState dst);
	bool CallExitState(LivingAction::Index index, LivingState dst);
	int CallOutOfAnimation(LivingAction::Index index);
	bool CallValidate(LivingAction::Index index);

	// State functions
	uint32_t InvalidState();
	uint32_t GoHome();
	bool ExitAtHome(LivingState dst);
	uint32_t Created();
	uint32_t DecideWhatToDo();
};
} // namespace openblack::entities::components
