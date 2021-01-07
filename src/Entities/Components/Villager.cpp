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
#include <functional>
#include <glm/common.hpp>
#include <random>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <string_view>

#include <glm/gtc/epsilon.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/string_cast.hpp>

#include "Abode.h"
#include "Common/MeshLookup.h"
#include "Entities/Registry.h"
#include "Game.h"
#include "Mesh.h"
#include "Mobile.h"
#include "Town.h"
#include "Transform.h"

using namespace openblack;
using namespace openblack::entities::components;

struct VillagerStateTableEntry
{
	std::function<uint32_t(Villager&)> state;
	std::function<bool(Villager&, LivingState, LivingState)> entryState;
	std::function<bool(Villager&, LivingState)> exitState;
	std::function<bool(Villager&)> saveState;
	std::function<bool(Villager&)> loadState;
	std::function<bool(Villager&)> field_0x50;
	std::function<bool(Villager&)> field_0x60;
	std::function<int(Villager&)> transitionAnimation;
	std::function<bool(Villager&)> validate;
};

static VillagerStateTableEntry TodoEntry = {
    .state = [](Villager& villager) -> uint32_t {
	    spdlog::get("ai")->warn("Villager #{}: TODO: Unimplemented state function: {}",
	                            Game::instance()->GetEntityRegistry().ToEntity(villager),
	                            LivingStateStrings[static_cast<size_t>(villager.GetState(Villager::LivingAction::Index::Top))]);
	    return 0;
    },
    .entryState = [](Villager& villager, LivingState src, LivingState dst) -> bool {
	    spdlog::get("ai")->warn("Villager #{}: TODO: Unimplemented entry state function ({} -> {})",
	                            Game::instance()->GetEntityRegistry().ToEntity(villager),
	                            LivingStateStrings[static_cast<size_t>(src)], LivingStateStrings[static_cast<size_t>(dst)]);
	    return false;
    },
    .exitState = [](Villager& villager, LivingState dst) -> bool {
	    spdlog::get("ai")->warn("Villager #{}: TODO: Unimplemented exit state function ({} -> {})",
	                            Game::instance()->GetEntityRegistry().ToEntity(villager),
	                            LivingStateStrings[static_cast<size_t>(villager.GetState(Villager::LivingAction::Index::Top))],
	                            LivingStateStrings[static_cast<size_t>(dst)]);
	    return false;
    },
    .saveState = [](Villager& villager) -> bool {
	    spdlog::get("ai")->warn("Villager #{}: TODO: Unimplemented save state function: {}",
	                            Game::instance()->GetEntityRegistry().ToEntity(villager),
	                            LivingStateStrings[static_cast<size_t>(villager.GetState(Villager::LivingAction::Index::Top))]);
	    return false;
    },
    .loadState = [](Villager& villager) -> bool {
	    spdlog::get("ai")->warn("Villager #{}: TODO: Unimplemented load state function: {}",
	                            Game::instance()->GetEntityRegistry().ToEntity(villager),
	                            LivingStateStrings[static_cast<size_t>(villager.GetState(Villager::LivingAction::Index::Top))]);
	    return false;
    },
    .field_0x50 = [](Villager& villager) -> bool {
	    spdlog::get("ai")->warn("Villager #{}: TODO: Unimplemented field_0x50 state function: {}",
	                            Game::instance()->GetEntityRegistry().ToEntity(villager),
	                            LivingStateStrings[static_cast<size_t>(villager.GetState(Villager::LivingAction::Index::Top))]);
	    return false;
    },
    .field_0x60 = [](Villager& villager) -> bool {
	    spdlog::get("ai")->warn("Villager #{}: TODO: Unimplemented field_0x60 state function: {}",
	                            Game::instance()->GetEntityRegistry().ToEntity(villager),
	                            LivingStateStrings[static_cast<size_t>(villager.GetState(Villager::LivingAction::Index::Top))]);
	    return false;
    },
    .transitionAnimation = [](Villager& villager) -> bool {
	    spdlog::get("ai")->warn("Villager #{}: TODO: Unimplemented transition animation function: {}",
	                            Game::instance()->GetEntityRegistry().ToEntity(villager),
	                            LivingStateStrings[static_cast<size_t>(villager.GetState(Villager::LivingAction::Index::Top))]);
	    return -1;
    },
    .validate = [](Villager& villager) -> bool {
	    spdlog::get("ai")->warn("Villager #{}: TODO: Unimplemented validate function: {}",
	                            Game::instance()->GetEntityRegistry().ToEntity(villager),
	                            LivingStateStrings[static_cast<size_t>(villager.GetState(Villager::LivingAction::Index::Top))]);
	    return false;
    },
};

std::array<VillagerStateTableEntry, static_cast<size_t>(LivingState::_COUNT)> VillagerStateTable = {
    /* INVALID_STATE */ VillagerStateTableEntry {
        .state = &Villager::InvalidState,
    },
    /* MOVE_TO_POS */ TodoEntry,
    /* MOVE_TO_OBJECT */ TodoEntry,
    /* MOVE_ON_STRUCTURE */ TodoEntry,
    /* IN_SCRIPT */ TodoEntry,
    /* IN_DANCE */ TodoEntry,
    /* FLEEING_FROM_OBJECT_REACTION */ TodoEntry,
    /* LOOKING_AT_OBJECT_REACTION */ TodoEntry,
    /* FOLLOWING_OBJECT_REACTION */ TodoEntry,
    /* INSPECT_OBJECT_REACTION */ TodoEntry,
    /* FLYING */ TodoEntry,
    /* LANDED */ TodoEntry,
    /* LOOK_AT_FLYING_OBJECT_REACTION */ TodoEntry,
    /* SET_DYING */ TodoEntry,
    /* DYING */ TodoEntry,
    /* DEAD */ TodoEntry,
    /* DROWNING */ TodoEntry,
    /* DOWNED */ TodoEntry,
    /* BEING_EATEN */ TodoEntry,
    /* GOTO_FOOD_REACTION */ TodoEntry,
    /* ARRIVES_AT_FOOD_REACTION */ TodoEntry,
    /* GOTO_WOOD_REACTION */ TodoEntry,
    /* ARRIVES_AT_WOOD_REACTION */ TodoEntry,
    /* WAIT_FOR_ANIMATION */ TodoEntry,
    /* IN_HAND */ TodoEntry,
    /* GOTO_PICKUP_BALL_REACTION */ TodoEntry,
    /* ARRIVES_AT_PICKUP_BALL_REACTION */ TodoEntry,
    /* MOVE_IN_FLOCK */ TodoEntry,
    /* MOVE_ALONG_PATH */ TodoEntry,
    /* MOVE_ON_PATH */ TodoEntry,
    /* FLEEING_AND_LOOKING_AT_OBJECT_REACTION */ TodoEntry,
    /* GOTO_STORAGE_PIT_FOR_DROP_OFF */ TodoEntry,
    /* ARRIVES_AT_STORAGE_PIT_FOR_DROP_OFF */ TodoEntry,
    /* GOTO_STORAGE_PIT_FOR_FOOD */ TodoEntry,
    /* ARRIVES_AT_STORAGE_PIT_FOR_FOOD */ TodoEntry,
    /* ARRIVES_AT_HOME_WITH_FOOD */ TodoEntry,
    /* GO_HOME */
    VillagerStateTableEntry {
        .state = &Villager::GoHome,
        .exitState = &Villager::ExitAtHome,
        .field_0x50 = TodoEntry.field_0x50,
    },
    /* ARRIVES_HOME */ TodoEntry,
    /* AT_HOME */ TodoEntry,
    /* ARRIVES_AT_STORAGE_PIT_FOR_BUILDING_MATERIALS */ TodoEntry,
    /* ARRIVES_AT_BUILDING_SITE */ TodoEntry,
    /* BUILDING */ TodoEntry,
    /* GOTO_STORAGE_PIT_FOR_WORSHIP_SUPPLIES */ TodoEntry,
    /* ARRIVES_AT_STORAGE_PIT_FOR_WORSHIP_SUPPLIES */ TodoEntry,
    /* GOTO_WORSHIP_SITE_WITH_SUPPLIES */ TodoEntry,
    /* MOVE_TO_WORSHIP_SITE_WITH_SUPPLIES */ TodoEntry,
    /* ARRIVES_AT_WORSHIP_SITE_WITH_SUPPLIES */ TodoEntry,
    /* FORESTER_MOVE_TO_FOREST */ TodoEntry,
    /* FORESTER_GOTO_FOREST */ TodoEntry,
    /* FORESTER_ARRIVES_AT_FOREST */ TodoEntry,
    /* FORESTER_CHOPS_TREE */ TodoEntry,
    /* FORESTER_CHOPS_TREE_FOR_BUILDING */ TodoEntry,
    /* FORESTER_FINISHED_FORESTERING */ TodoEntry,
    /* ARRIVES_AT_BIG_FOREST */ TodoEntry,
    /* ARRIVES_AT_BIG_FOREST_FOR_BUILDING */ TodoEntry,
    /* FISHERMAN_ARRIVES_AT_FISHING */ TodoEntry,
    /* FISHING */ TodoEntry,
    /* WAIT_FOR_COUNTER */ TodoEntry,
    /* GOTO_WORSHIP_SITE_FOR_WORSHIP */ TodoEntry,
    /* ARRIVES_AT_WORSHIP_SITE_FOR_WORSHIP */ TodoEntry,
    /* WORSHIPPING_AT_WORSHIP_SITE */ TodoEntry,
    /* GOTO_ALTAR_FOR_REST */ TodoEntry,
    /* ARRIVES_AT_ALTAR_FOR_REST */ TodoEntry,
    /* AT_ALTAR_REST */ TodoEntry,
    /* AT_ALTAR_FINISHED_REST */ TodoEntry,
    /* RESTART_WORSHIPPING_AT_WORSHIP_SITE */ TodoEntry,
    /* RESTART_WORSHIPPING_CREATURE */ TodoEntry,
    /* FARMER_ARRIVES_AT_FARM */ TodoEntry,
    /* FARMER_PLANTS_CROP */ TodoEntry,
    /* FARMER_DIGS_UP_CROP */ TodoEntry,
    /* MOVE_TO_FOOTBALL_PITCH_CONSTRUCTION */ TodoEntry,
    /* FOOTBALL_WALK_TO_POSITION */ TodoEntry,
    /* FOOTBALL_WAIT_FOR_KICK_OFF */ TodoEntry,
    /* FOOTBALL_ATTACKER */ TodoEntry,
    /* FOOTBALL_GOALIE */ TodoEntry,
    /* FOOTBALL_DEFENDER */ TodoEntry,
    /* FOOTBALL_WON_GOAL */ TodoEntry,
    /* FOOTBALL_LOST_GOAL */ TodoEntry,
    /* START_MOVE_TO_PICK_UP_BALL_FOR_DEAD_BALL */ TodoEntry,
    /* ARRIVED_AT_PICK_UP_BALL_FOR_DEAD_BALL */ TodoEntry,
    /* ARRIVED_AT_PUT_DOWN_BALL_FOR_DEAD_BALL_START */ TodoEntry,
    /* ARRIVED_AT_PUT_DOWN_BALL_FOR_DEAD_BALL_END */ TodoEntry,
    /* FOOTBALL_MATCH_PAUSED */ TodoEntry,
    /* FOOTBALL_WATCH_MATCH */ TodoEntry,
    /* FOOTBALL_MEXICAN_WAVE */ TodoEntry,
    /* CREATED */
    VillagerStateTableEntry {
        .state = &Villager::Created,
        .field_0x50 = TodoEntry.field_0x50,
    },
    /* ARRIVES_IN_ABODE_TO_TRADE */ TodoEntry,
    /* ARRIVES_IN_ABODE_TO_PICK_UP_EXCESS */ TodoEntry,
    /* MAKE_SCARED_STIFF */ TodoEntry,
    /* SCARED_STIFF */ TodoEntry,
    /* WORSHIPPING_CREATURE */ TodoEntry,
    /* SHEPHERD_LOOK_FOR_FLOCK */ TodoEntry,
    /* SHEPHERD_MOVE_FLOCK_TO_WATER */ TodoEntry,
    /* SHEPHERD_MOVE_FLOCK_TO_FOOD */ TodoEntry,
    /* SHEPHERD_MOVE_FLOCK_BACK */ TodoEntry,
    /* SHEPHERD_DECIDE_WHAT_TO_DO_WITH_FLOCK */ TodoEntry,
    /* SHEPHERD_WAIT_FOR_FLOCK */ TodoEntry,
    /* SHEPHERD_SLAUGHTER_ANIMAL */ TodoEntry,
    /* SHEPHERD_FETCH_STRAY */ TodoEntry,
    /* SHEPHERD_GOTO_FLOCK */ TodoEntry,
    /* HOUSEWIFE_AT_HOME */ TodoEntry,
    /* HOUSEWIFE_GOTO_STORAGE_PIT */ TodoEntry,
    /* HOUSEWIFE_ARRIVES_AT_STORAGE_PIT */ TodoEntry,
    /* HOUSEWIFE_PICKUP_FROM_STORAGE_PIT */ TodoEntry,
    /* HOUSEWIFE_RETURN_HOME_WITH_FOOD */ TodoEntry,
    /* HOUSEWIFE_MAKE_DINNER */ TodoEntry,
    /* HOUSEWIFE_SERVES_DINNER */ TodoEntry,
    /* HOUSEWIFE_CLEARS_AWAY_DINNER */ TodoEntry,
    /* HOUSEWIFE_DOES_HOUSEWORK */ TodoEntry,
    /* HOUSEWIFE_GOSSIPS_AROUND_STORAGE_PIT */ TodoEntry,
    /* HOUSEWIFE_STARTS_GIVING_BIRTH */ TodoEntry,
    /* HOUSEWIFE_GIVING_BIRTH */ TodoEntry,
    /* HOUSEWIFE_GIVEN_BIRTH */ TodoEntry,
    /* CHILD_AT_CRECHE */ TodoEntry,
    /* CHILD_FOLLOWS_MOTHER */ TodoEntry,
    /* CHILD_BECOMES_ADULT */ TodoEntry,
    /* SITS_DOWN_TO_DINNER */ TodoEntry,
    /* EAT_FOOD */ TodoEntry,
    /* EAT_FOOD_AT_HOME */ TodoEntry,
    /* GOTO_BED_AT_HOME */ TodoEntry,
    /* SLEEPING_AT_HOME */ TodoEntry,
    /* WAKE_UP_AT_HOME */ TodoEntry,
    /* START_HAVING_SEX */ TodoEntry,
    /* HAVING_SEX */ TodoEntry,
    /* STOP_HAVING_SEX */ TodoEntry,
    /* START_HAVING_SEX_AT_HOME */ TodoEntry,
    /* HAVING_SEX_AT_HOME */ TodoEntry,
    /* STOP_HAVING_SEX_AT_HOME */ TodoEntry,
    /* WAIT_FOR_DINNER */ TodoEntry,
    /* HOMELESS_START */ TodoEntry,
    /* VAGRANT_START */ TodoEntry,
    /* MORN_DEATH */ TodoEntry,
    /* PERFORM_INSPECTION_REACTION */ TodoEntry,
    /* APPROACH_OBJECT_REACTION */ TodoEntry,
    /* INITIALISE_TELL_OTHERS_ABOUT_OBJECT */ TodoEntry,
    /* TELL_OTHERS_ABOUT_INTERESTING_OBJECT */ TodoEntry,
    /* APPROACH_VILLAGER_TO_TALK_TO */ TodoEntry,
    /* TELL_PARTICULAR_VILLAGER_ABOUT_OBJECT */ TodoEntry,
    /* INITIALISE_LOOK_AROUND_FOR_VILLAGER_TO_TELL */ TodoEntry,
    /* LOOK_AROUND_FOR_VILLAGER_TO_TELL */ TodoEntry,
    /* MOVE_TOWARDS_OBJECT_TO_LOOK_AT */ TodoEntry,
    /* INITIALISE_IMPRESSED_REACTION */ TodoEntry,
    /* PERFORM_IMPRESSED_REACTION */ TodoEntry,
    /* INITIALISE_FIGHT_REACTION */ TodoEntry,
    /* PERFORM_FIGHT_REACTION */ TodoEntry,
    /* HOMELESS_EAT_DINNER */ TodoEntry,
    /* INSPECT_CREATURE_REACTION */ TodoEntry,
    /* PERFORM_INSPECT_CREATURE_REACTION */ TodoEntry,
    /* APPROACH_CREATURE_REACTION */ TodoEntry,
    /* INITIALISE_BEWILDERED_BY_MAGIC_TREE_REACTION */ TodoEntry,
    /* PERFORM_BEWILDERED_BY_MAGIC_TREE_REACTION */ TodoEntry,
    /* TURN_TO_FACE_MAGIC_TREE */ TodoEntry,
    /* LOOK_AT_MAGIC_TREE */ TodoEntry,
    /* DANCE_FOR_EDITING_PURPOSES */ TodoEntry,
    /* MOVE_TO_DANCE_POS */ TodoEntry,
    /* INITIALISE_RESPECT_CREATURE_REACTION */ TodoEntry,
    /* PERFORM_RESPECT_CREATURE_REACTION */ TodoEntry,
    /* FINISH_RESPECT_CREATURE_REACTION */ TodoEntry,
    /* APPROACH_HAND_REACTION */ TodoEntry,
    /* FLEEING_FROM_CREATURE_REACTION */ TodoEntry,
    /* TURN_TO_FACE_CREATURE_REACTION */ TodoEntry,
    /* WATCH_FLYING_OBJECT_REACTION */ TodoEntry,
    /* POINT_AT_FLYING_OBJECT_REACTION */ TodoEntry,
    /* DECIDE_WHAT_TO_DO */
    VillagerStateTableEntry {
        .state = &Villager::DecideWhatToDo,
        .field_0x50 = TodoEntry.field_0x50,
    },
    /* INTERACT_DECIDE_WHAT_TO_DO */ TodoEntry,
    /* EAT_OUTSIDE */ TodoEntry,
    /* RUN_AWAY_FROM_OBJECT_REACTION */ TodoEntry,
    /* MOVE_TOWARDS_CREATURE_REACTION */ TodoEntry,
    /* AMAZED_BY_MAGIC_SHIELD_REACTION */ TodoEntry,
    /* VILLAGER_GOSSIPS */ TodoEntry,
    /* CHECK_INTERACT_WITH_ANIMAL */ TodoEntry,
    /* CHECK_INTERACT_WITH_WORSHIP_SITE */ TodoEntry,
    /* CHECK_INTERACT_WITH_ABODE */ TodoEntry,
    /* CHECK_INTERACT_WITH_FIELD */ TodoEntry,
    /* CHECK_INTERACT_WITH_FISH_FARM */ TodoEntry,
    /* CHECK_INTERACT_WITH_TREE */ TodoEntry,
    /* CHECK_INTERACT_WITH_BALL */ TodoEntry,
    /* CHECK_INTERACT_WITH_POT */ TodoEntry,
    /* CHECK_INTERACT_WITH_FOOTBALL */ TodoEntry,
    /* CHECK_INTERACT_WITH_VILLAGER */ TodoEntry,
    /* CHECK_INTERACT_WITH_MAGIC_LIVING */ TodoEntry,
    /* CHECK_INTERACT_WITH_ROCK */ TodoEntry,
    /* ARRIVES_AT_ROCK_FOR_WOOD */ TodoEntry,
    /* GOT_WOOD_FROM_ROCK */ TodoEntry,
    /* REENTER_BUILDING_STATE */ TodoEntry,
    /* ARRIVE_AT_PUSH_OBJECT */ TodoEntry,
    /* TAKE_WOOD_FROM_TREE */ TodoEntry,
    /* TAKE_WOOD_FROM_POT */ TodoEntry,
    /* TAKE_WOOD_FROM_TREE_FOR_BUILDING */ TodoEntry,
    /* TAKE_WOOD_FROM_POT_FOR_BUILDING */ TodoEntry,
    /* SHEPHERD_TAKE_ANIMAL_FOR_SLAUGHTER */ TodoEntry,
    /* SHEPHERD_TAKES_CONTROL_OF_FLOCK */ TodoEntry,
    /* SHEPHERD_RELEASES_CONTROL_OF_FLOCK */ TodoEntry,
    /* DANCE_BUT_NOT_WORSHIP */ TodoEntry,
    /* FAINTING_REACTION */ TodoEntry,
    /* START_CONFUSED_REACTION */ TodoEntry,
    /* CONFUSED_REACTION */ TodoEntry,
    /* AFTER_TAP_ON_ABODE */ TodoEntry,
    /* WEAK_ON_GROUND */ TodoEntry,
    /* SCRIPT_WANDER_AROUND_POSITION */ TodoEntry,
    /* SCRIPT_PLAY_ANIM */ TodoEntry,
    /* GO_TOWARDS_TELEPORT_REACTION */ TodoEntry,
    /* TELEPORT_REACTION */ TodoEntry,
    /* DANCE_WHILE_REACTING */ TodoEntry,
    /* CONTROLLED_BY_CREATURE */ TodoEntry,
    /* POINT_AT_DEAD_PERSON */ TodoEntry,
    /* GO_TOWARDS_DEAD_PERSON */ TodoEntry,
    /* LOOK_AT_DEAD_PERSON */ TodoEntry,
    /* MOURN_DEAD_PERSON */ TodoEntry,
    /* NOTHING_TO_DO */ TodoEntry,
    /* ARRIVES_AT_WORKSHOP_FOR_DROP_OFF */ TodoEntry,
    /* ARRIVES_AT_STORAGE_PIT_FOR_WORKSHOP_MATERIALS */ TodoEntry,
    /* SHOW_POISONED */ TodoEntry,
    /* HIDING_AT_WORSHIP_SITE */ TodoEntry,
    /* CROWD_REACTION */ TodoEntry,
    /* REACT_TO_FIRE */ TodoEntry,
    /* PUT_OUT_FIRE_BY_BEATING */ TodoEntry,
    /* PUT_OUT_FIRE_WITH_WATER */ TodoEntry,
    /* GET_WATER_TO_PUT_OUT_FIRE */ TodoEntry,
    /* ON_FIRE */ TodoEntry,
    /* MOVE_AROUND_FIRE */ TodoEntry,
    /* DISCIPLE_NOTHING_TO_DO */ TodoEntry,
    /* FOOTBALL_MOVE_TO_BALL */ TodoEntry,
    /* ARRIVES_AT_STORAGE_PIT_FOR_TRADER_PICK_UP */ TodoEntry,
    /* ARRIVES_AT_STORAGE_PIT_FOR_TRADER_DROP_OFF */ TodoEntry,
    /* BREEDER_DISCIPLE */ TodoEntry,
    /* MISSIONARY_DISCIPLE */ TodoEntry,
    /* REACT_TO_BREEDER */ TodoEntry,
    /* SHEPHERD_CHECK_ANIMAL_FOR_SLAUGHTER */ TodoEntry,
    /* INTERACT_DECIDE_WHAT_TO_DO_FOR_OTHER_VILLAGER */ TodoEntry,
    /* ARTIFACT_DANCE */ TodoEntry,
    /* FLEEING_FROM_PREDATOR_REACTION */ TodoEntry,
    /* WAIT_FOR_WOOD */ TodoEntry,
    /* INSPECT_OBJECT */ TodoEntry,
    /* GO_HOME_AND_CHANGE */ TodoEntry,
    /* WAIT_FOR_MATE */ TodoEntry,
    /* GO_AND_HIDE_IN_NEARBY_BUILDING */ TodoEntry,
    /* LOOK_TO_SEE_IF_IT_IS_SAFE */ TodoEntry,
    /* SLEEP_IN_TENT */ TodoEntry,
    /* PAUSE_FOR_A_SECOND */ TodoEntry,
    /* PANIC_REACTION */ TodoEntry,
    /* GET_FOOD_AT_WORSHIP_SITE */ TodoEntry,
    /* GOTO_CONGREGATE_IN_TOWN_AFTER_EMERGENCY */ TodoEntry,
    /* CONGREGATE_IN_TOWN_AFTER_EMERGENCY */ TodoEntry,
    /* SCRIPT_IN_CROWD */ TodoEntry,
    /* GO_AND_CHILLOUT_OUTSIDE_HOME */ TodoEntry,
    /* SIT_AND_CHILLOUT */ TodoEntry,
    /* SCRIPT_GO_AND_MOVE_ALONG_PATH */ TodoEntry,
    /* RESTART_MEETING */ TodoEntry,
    /* GOTO_ABODE_BURNING_REACTION */ TodoEntry,
    /* ARRIVES_AT_ABODE_BURNING_REACTION */ TodoEntry,
    /* REPAIRS_ABODE */ TodoEntry,
    /* ARRIVES_AT_SCAFFOLD_FOR_PICKUP */ TodoEntry,
    /* ARRIVES_AT_BUILDING_SITE_WITH_SCAFFOLD */ TodoEntry,
    /* MOVE_SCAFFOLD_TO_BUILDING_SITE */ TodoEntry,
};

void Villager::Create(const glm::vec3& abodePosition, const glm::vec3& position, Tribe tribe, Role role, uint32_t age)
{
	// TODO(bwrsandman): would be nice to have this be a non-static member of
	//  FeatureScriptCommands or use a singleton
	const auto seed = 0x1000;
	thread_local std::default_random_engine generator(seed);
	thread_local std::uniform_int_distribution<uint16_t> duration_distribution(1, 500);

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
	registry.Assign<MobileWallHug>(entity);
	const uint32_t health = 100;
	const uint32_t hunger = 100;
	const auto lifeStage = age >= 18 ? Villager::LifeStage::Adult : Villager::LifeStage::Child;
	const auto sex = (role == Villager::Role::HOUSEWIFE) ? Villager::Sex::FEMALE : Villager::Sex::MALE;
	const auto task = Villager::Task::IDLE;
	const auto turnsUntilStateChange = duration_distribution(generator);
	const auto action = LivingAction {{LivingState::CREATED}, turnsUntilStateChange, 0};
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

uint32_t Villager::CallState(LivingAction::Index index)
{
	const auto& state = livingAction.states[static_cast<size_t>(index)];
	const auto& entry = VillagerStateTable[static_cast<size_t>(state)];
	const auto& callback = entry.state;
	if (!callback)
	{
		return 0;
	}
	return callback(*this);
}

bool Villager::CallEntryState(LivingAction::Index index, LivingState src, LivingState dst)
{
	const auto& state = livingAction.states[static_cast<size_t>(index)];
	const auto& entry = VillagerStateTable[static_cast<size_t>(state)];
	const auto& callback = entry.entryState;
	if (!callback)
	{
		return true;
	}
	return callback(*this, src, dst);
}

bool Villager::CallExitState(LivingAction::Index index, LivingState dst)
{
	const auto& state = livingAction.states[static_cast<size_t>(index)];
	const auto& entry = VillagerStateTable[static_cast<size_t>(state)];
	const auto& callback = entry.exitState;
	if (!callback)
	{
		return true;
	}
	return callback(*this, dst);
}

int Villager::CallOutOfAnimation(LivingAction::Index index)
{
	const auto& state = livingAction.states[static_cast<size_t>(index)];
	const auto& entry = VillagerStateTable[static_cast<size_t>(state)];
	const auto& callback = entry.transitionAnimation;
	if (!callback)
	{
		return false;
	}
	return callback(*this);
}

bool Villager::CallValidate(LivingAction::Index index)
{
	const auto& state = livingAction.states[static_cast<size_t>(index)];
	const auto& entry = VillagerStateTable[static_cast<size_t>(state)];
	const auto& callback = entry.validate;
	if (!callback)
	{
		return false;
	}
	return callback(*this);
}

void Villager::SetState(LivingAction::Index index, LivingState state, bool skipTransition)
{
	assert(index < LivingAction::Index::_Count);
	if (livingAction.states[static_cast<size_t>(index)] != state)
	{
		const auto previousState = livingAction.states[static_cast<size_t>(index)];
		livingAction.states[static_cast<size_t>(index)] = state;
		spdlog::debug("Villager #{}: Setting state {} -> {}", Game::instance()->GetEntityRegistry().ToEntity(*this),
		              LivingStateStrings[static_cast<size_t>(previousState)], LivingStateStrings[static_cast<size_t>(state)]);
		if (index == LivingAction::Index::Top)
		{
			livingAction.turnsSinceStateChange = 0;
			if (skipTransition)
			{
				return;
			}

			if (CallExitState(index, state))
			{
				return;
			}

			// auto animResult = CallOutOfAnimation(index);

			CallEntryState(index, previousState, state);

			// SetStateSpeed();
			//
			// if (animResult != -1)
			// {
			// 	SetAnim(animResult, 1);
			// 	return;
			// }
			//
			// SetStateAnim();
			//
			// animResult = CallIntoAnimation(destination)
			// if (animResult != -1) {
			//     SetAnim(animResult, 1);
			//     return;
			// }
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

std::optional<glm::vec3> Villager::GetChillOutPos()
{
	thread_local std::default_random_engine generator;
	auto angleDistribution = std::uniform_real_distribution<float>(glm::pi<float>() / 4.0f);

	auto town = GetTown();
	if (!town.has_value())
	{
		return std::nullopt;
	}

	// TODO(bwrsandman): get town chillout radius (town->super.info->field_0x140)
	float townChillOutRadius = 1.0f;
	auto radiusDistribution = std::uniform_real_distribution<float>(townChillOutRadius);
	float randomizedRadius = glm::mix(townChillOutRadius, radiusDistribution(generator), 0.9f);

	const auto& registry = Game::instance()->GetEntityRegistry();

	const auto transform = registry.As<Transform>(*this);
	const auto& position = transform.position;
	const auto& congregationPos = town->get().GetCongregationPos();

	// TODO(bwrsandman): get xz angle between two positions
	float angle = 0.0f;
	// Add turn up to -22.5 degrees
	angle += -glm::pi<float>() / 8.0f;
	// Add random town up to 45 degrees
	angle += angleDistribution(generator);

	// TODO(bwrsandman): get position from randomizedRadius and angle
	auto offset = glm::zero<glm::vec3>();

	return std::make_optional<glm::vec3>(congregationPos + offset);
}

bool Villager::GetTentPos(glm::vec3& position)
{
	assert(false);
}

bool Villager::SetupMoveToWithHug(const glm::vec3& coords, LivingState destinationState)
{
	spdlog::get("ai")->debug("Villager #{}: Setting up a move to {} to then switch to {}",
	                         Game::instance()->GetEntityRegistry().ToEntity(*this), glm::to_string(coords),
	                         LivingStateStrings[static_cast<size_t>(destinationState)]);

	// TODO(bwrsandman): Info could have been overridden by a superclass in vanilla
	// TODO(bwrsandman): Info may be better as a component
	auto info = Game::instance()->GetInfoConstants().GetVillagerInfo(GetVillagerType());
	if (!info.has_value())
	{
		assert(false);
		return false;
	}
	auto currentState = info->get().moveToPosState;
	if (!CallExitState(LivingAction::Index::Top, destinationState))
	{
		spdlog::get("ai")->error("Villager #{}: Unable to setup a move to {}: call to exit from {} into {} state failed",
		                         Game::instance()->GetEntityRegistry().ToEntity(*this), glm::to_string(coords),
		                         LivingStateStrings[static_cast<size_t>(currentState)],
		                         LivingStateStrings[static_cast<size_t>(destinationState)]);
		return false;
	}

	// int iVar1 = CallOutofAnimationFunction(destinationState);

	if (CallEntryState(LivingAction::Index::Top, currentState, destinationState))
	{
		// SetStateSpeed();
		// if (iVar1 != 1)
		// {
		//     SetAnim(iVar1, 1);
		// }

		auto& wallHug = Game::instance()->GetEntityRegistry().As<entities::components::MobileWallHug>(*this);
		wallHug.SetupMobileMoveToPos(coords, MoveToState::_0x0c);
		return true;
	}

	spdlog::get("ai")->error("Villager #{}: Unable to setup a move to {}: call to entry from {} into {} state failed",
	                         Game::instance()->GetEntityRegistry().ToEntity(*this), glm::to_string(coords),
	                         LivingStateStrings[static_cast<size_t>(currentState)],
	                         LivingStateStrings[static_cast<size_t>(destinationState)]);

	CallEntryState(LivingAction::Index::Top, currentState, LivingState::DECIDE_WHAT_TO_DO);
	return false;
}

bool Villager::SetupMoveToOnFootpath(const Abode& destination, const glm::vec3& coords, LivingState state)
{
	const auto& registry = Game::instance()->GetEntityRegistry();
	const auto& position = registry.As<Transform>(*this).position;
	if (glm::all(glm::epsilonEqual(position, destination.GetDoorOffset(), glm::epsilon<float>())))
	{
		if (glm::all(glm::epsilonEqual(position, coords, glm::epsilon<float>())))
		{
			return SetupMoveToWithHug(coords, state);
		}
	}
	return registry.As<Transform>(destination).UseFootpathIfNecessary(*this, coords, state);
}

bool Villager::SetupMoveOnFootpath(const Footpath& footpath, const glm::vec3& coords, LivingState state)
{
	// get current node from *this
	// remove *this from current node's follower list
	// update current node based on Transform(this)->position
	// add *this to new current node
	// call SetupMobileMoveToPos
	// call virtual function 0x8dc and if returns true, call 0x8fc and return true
	assert(false);
}

uint32_t Villager::DoGoingHome(LivingState state, LivingState homelessState)
{
	LivingState newState;

	// TODO: If the villager is dancing, remove from their dance

	const auto home = GetAbode();
	if (home.has_value())
	{
		// TODO: if flag's 3rd (0b100, likely means at home) bit is not set which is often
		// if ((*(byte *)&this->field_0xe0 & 4) == 0)
		{
			if (GetState(LivingAction::Index::Final) != state)
			{
				SetupMoveToOnFootpath(home->get(), home->get().GetDoorOffset(), state);
			}
			return 1;
		}
		newState = LivingState::AT_HOME;
	}
	else
	{
		if (town.has_value())
		{
			const auto& registry = Game::instance()->GetEntityRegistry();
			const auto position = registry.As<Transform>(*this).position;
			const auto townPosition = registry.As<Transform>(*town).position;
			const float distanceThreshold = 100.0f;
			const auto distance2 = glm::distance2(position, townPosition);

			auto goal = position;

			// TODO use a random service
			thread_local std::default_random_engine generator;

			if (distance2 > distanceThreshold * distanceThreshold)
			{
				homelessState = GetState(LivingAction::Index::Top);
				auto angleDistribution =
				    std::uniform_real_distribution<float>(-glm::pi<float>() / 4.0f, glm::pi<float>() / 4.0f);
				auto radiusDistribution = std::uniform_real_distribution<float>(10.0f, 35.0f);
				const auto angle =
				    angleDistribution(generator) + glm::atan(position.x - townPosition.x, position.z - townPosition.z);
				const auto radius = angleDistribution(generator);
				goal += glm::vec3(glm::cos(angle) * radius, 0.0f, glm::sin(angle) * radius);
			}
			else
			{
				auto angleDistribution = std::uniform_real_distribution<float>(glm::pi<float>() * 2.0f);
				{
					auto radiusDistribution = std::uniform_real_distribution<float>(2.0f, 10.0f);
					const auto angle = angleDistribution(generator);
					const auto radius = angleDistribution(generator);
					goal += glm::vec3(glm::cos(angle) * radius, 0.0f, glm::sin(angle) * radius);
				}

				if (!GetTentPos(goal))
				{
					homelessState = GetState(LivingAction::Index::Top);
					auto radiusDistribution = std::uniform_real_distribution<float>(10.0f, 30.0f);
					const auto angle = angleDistribution(generator);
					const auto radius = angleDistribution(generator);
					goal += glm::vec3(glm::cos(angle) * radius, 0.0f, glm::sin(angle) * radius);
				}
			}
			SetupMoveToWithHug(goal, homelessState);
			return 1;
		}
		newState = LivingState::VAGRANT_START;
	}
	SetState(LivingAction::Index::Top, newState, false);
	return 1;
}

void Villager::LeaveHome()
{
	assert(false);
}

uint32_t Villager::InvalidState()
{
	spdlog::error("Villager #{}: Stuck in an invalid state", Game::instance()->GetEntityRegistry().ToEntity(*this));
	assert(false);
	return 0;
}

uint32_t Villager::GoHome()
{
	return DoGoingHome(LivingState::ARRIVES_HOME, LivingState::SLEEP_IN_TENT);
}

bool Villager::ExitAtHome(LivingState dst)
{
	if (Game::instance()->GetInfoConstants().GetVillagerStateInfo(dst).field_0xd0)
	{
		LeaveHome();
	}
	return true;
}

uint32_t Villager::Created()
{
	if (livingAction.turnsUntilStateChange > 0)
	{
		--livingAction.turnsUntilStateChange;
	}
	else
	{
		SetState(LivingAction::Index::Top, LivingState::DECIDE_WHAT_TO_DO, false);
	}
	return 0;
}

uint32_t Villager::DecideWhatToDo()
{
	// TODO(bwrsandman): Find town of villager and check for emergency -> GOTO_CONGREGATE_IN_TOWN_AFTER_EMERGENCY
	// TODO(bwrsandman): Check if villager is disciple and use disciple table or unassign disciple
	// TODO(bwrsandman): Check if villager is child which is a different decision tree
	// TODO(bwrsandman): Check if needed for something (log this with spdlog::debug)
	// TODO(bwrsandman): Check if needs resources (log this with spdlog::debug)
	// else
	thread_local std::default_random_engine generator;
	thread_local std::uniform_int_distribution<uint8_t> distributionChoice(0, 9);
	thread_local std::uniform_int_distribution<uint8_t> distributionGoToNonFunctionHome(0, 100);

	const auto& town = GetTown();
	const auto& abode = GetAbode();

	const auto choice = distributionChoice(generator);
	switch (choice)
	{
	case 0:
		if (abode.has_value() && (abode->get().IsFunctional() || distributionGoToNonFunctionHome(generator) < 10))
		{
			SetState(LivingAction::Index::Top, LivingState::GO_HOME, false);
			return 1;
		}
		[[fallthrough]];
	case 1:
	case 2:
	case 3:
		if (abode.has_value())
		{
			SetState(LivingAction::Index::Top, LivingState::GO_AND_CHILLOUT_OUTSIDE_HOME, false);
			return 1;
		}
		[[fallthrough]];
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	{
		auto coords = GetChillOutPos();
		if (coords.has_value())
		{
			SetupMoveToWithHug(coords.value(), LivingState::SIT_AND_CHILLOUT);
			return 1;
		}
		[[fallthrough]];
	}
	default:
		SetState(LivingAction::Index::Top, LivingState::GO_HOME, false);
		return 1;
	}
}
