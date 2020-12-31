/*****************************************************************************
 * Copyright (c) 2018-2021 openblack developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/openblack/openblack
 *
 * openblack is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#include "FeatureScriptCommands.h"

#include <tuple>

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/string_cast.hpp>
#include <spdlog/spdlog.h>

#include "3D/Camera.h"
#include "3D/LandIsland.h"
#include "AllMeshes.h"
#include "Entities/Components/Abode.h"
#include "Entities/Components/AnimatedStatic.h"
#include "Entities/Components/Field.h"
#include "Entities/Components/Fixed.h"
#include "Entities/Components/Footpath.h"
#include "Entities/Components/Forest.h"
#include "Entities/Components/Mesh.h"
#include "Entities/Components/Stream.h"
#include "Entities/Components/Town.h"
#include "Entities/Components/Transform.h"
#include "Entities/Components/Tree.h"
#include "Entities/Components/Villager.h"
#include "Entities/Registry.h"
#include "Enums.h"
#include "Game.h"
#include "ScriptingBindingUtils.h"

using namespace openblack;
using namespace openblack::lhscriptx;
using namespace openblack::entities::components;

// alias parameter types for signature list readability
const constexpr ParameterType TString = ParameterType::String;
const constexpr ParameterType TNumber = ParameterType::Number;
const constexpr ParameterType TFloat = ParameterType::Float;
const constexpr ParameterType TVector = ParameterType::Vector;

namespace
{
std::unordered_map<std::string, Feature::Info> featureInfoLookup {
    {"Fat Pilar Lime", Feature::Info::FatPilarLime},
    {"Pilar3 Lime", Feature::Info::Pilar3Lime},
    {"Aztec Statue Feature", Feature::Info::AztcStatue},
    {"Spikey Pilar Lime", Feature::Info::SpikeyPilarLime},
    {"Pilar2 Lime", Feature::Info::Pilar2Lime},
    {"Crater", Feature::Info::Crater},
    {"Pier", Feature::Info::Pier},
};

const auto tribeLookup = makeLookup<Tribe>(TribeStrs);
const auto villagerRoleLookup = makeLookup<Villager::Role>(Villager::RoleStrs);

std::tuple<Tribe, Villager::Role> GetVillagerTribeAndRole(const std::string& villagerTribeWithType)
{
	const auto pos = villagerTribeWithType.find_first_of('_');
	const auto tribeStr = villagerTribeWithType.substr(0, pos);
	const auto roleStr = villagerTribeWithType.substr(pos + 1);

	try
	{
		const auto tribe = tribeLookup.at(tribeStr);
		const auto role = villagerRoleLookup.at(roleStr);
		return std::make_tuple(tribe, role);
	}
	catch (...)
	{
		std::throw_with_nested(std::runtime_error("Could not recognize either villager tribe or role"));
	}
}

} // namespace

const std::array<const ScriptCommandSignature, 105> FeatureScriptCommands::Signatures = {{
    CREATE_COMMAND_BINDING("CREATE_MIST", CreateMist),
    CREATE_COMMAND_BINDING("CREATE_PATH", CreatePath),
    CREATE_COMMAND_BINDING("CREATE_TOWN", CreateTown),
    CREATE_COMMAND_BINDING("SET_TOWN_BELIEF", SetTownBelief),
    CREATE_COMMAND_BINDING("SET_TOWN_BELIEF_CAP", SetTownBeliefCap),
    CREATE_COMMAND_BINDING("SET_TOWN_UNINHABITABLE", SetTownUninhabitable),
    CREATE_COMMAND_BINDING("SET_TOWN_CONGREGATION_POS", SetTownCongregationPos),
    CREATE_COMMAND_BINDING("CREATE_ABODE", CreateAbode),
    CREATE_COMMAND_BINDING("CREATE_PLANNED_ABODE", CreatePlannedAbode),
    CREATE_COMMAND_BINDING("CREATE_TOWN_CENTRE", CreateTownCentre),
    CREATE_COMMAND_BINDING("CREATE_TOWN_SPELL", CreateTownSpell),
    CREATE_COMMAND_BINDING("CREATE_NEW_TOWN_SPELL", CreateNewTownSpell),
    CREATE_COMMAND_BINDING("CREATE_TOWN_CENTRE_SPELL_ICON", CreateTownCentreSpellIcon),
    CREATE_COMMAND_BINDING("CREATE_SPELL_ICON", CreateSpellIcon),
    CREATE_COMMAND_BINDING("CREATE_PLANNED_SPELL_ICON", CreatePlannedSpellIcon),
    CREATE_COMMAND_BINDING("CREATE_VILLAGER", CreateVillager),
    CREATE_COMMAND_BINDING("CREATE_TOWN_VILLAGER", CreateTownVillager),
    CREATE_COMMAND_BINDING("CREATE_SPECIAL_TOWN_VILLAGER", CreateSpecialTownVillager),
    CREATE_COMMAND_BINDING("CREATE_VILLAGER_POS", CreateVillagerPos),
    CREATE_COMMAND_BINDING("CREATE_CITADEL", CreateCitadel),
    CREATE_COMMAND_BINDING("CREATE_PLANNED_CITADEL", CreatePlannedCitadel),
    CREATE_COMMAND_BINDING("CREATE_CREATURE_PEN", CreateCreaturePen),
    CREATE_COMMAND_BINDING("CREATE_WORSHIP_SITE", CreateWorshipSite),
    CREATE_COMMAND_BINDING("CREATE_PLANNED_WORSHIP_SITE", CreatePlannedWorshipSite),
    CREATE_COMMAND_BINDING("CREATE_ANIMAL", CreateAnimal),
    CREATE_COMMAND_BINDING("CREATE_NEW_ANIMAL", CreateNewAnimal),
    CREATE_COMMAND_BINDING("CREATE_FOREST", CreateForest),
    CREATE_COMMAND_BINDING("CREATE_TREE", CreateTree),
    CREATE_COMMAND_BINDING("CREATE_NEW_TREE", CreateNewTree),
    CREATE_COMMAND_BINDING("CREATE_FIELD", CreateField),
    CREATE_COMMAND_BINDING("CREATE_TOWN_FIELD", CreateTownField),
    CREATE_COMMAND_BINDING("CREATE_FISH_FARM", CreateFishFarm),
    CREATE_COMMAND_BINDING("CREATE_TOWN_FISH_FARM", CreateTownFishFarm),
    CREATE_COMMAND_BINDING("CREATE_FEATURE", CreateFeature),
    CREATE_COMMAND_BINDING("CREATE_FLOWERS", CreateFlowers),
    CREATE_COMMAND_BINDING("CREATE_WALL_SECTION", CreateWallSection),
    CREATE_COMMAND_BINDING("CREATE_PLANNED_WALL_SECTION", CreatePlannedWallSection),
    CREATE_COMMAND_BINDING("CREATE_PITCH", CreatePitch),
    CREATE_COMMAND_BINDING("CREATE_POT", CreatePot),
    CREATE_COMMAND_BINDING("CREATE_TOWN_TEMPORARY_POTS", CreateTownTemporaryPots),
    CREATE_COMMAND_BINDING("CREATE_MOBILEOBJECT", CreateMobileObject),
    CREATE_COMMAND_BINDING("CREATE_MOBILESTATIC", CreateMobileStatic),
    CREATE_COMMAND_BINDING("CREATE_MOBILE_STATIC", CreateMobileUStatic),
    CREATE_COMMAND_BINDING("CREATE_DEAD_TREE", CreateDeadTree),
    CREATE_COMMAND_BINDING("CREATE_SCAFFOLD", CreateScaffold),
    CREATE_COMMAND_BINDING("COUNTRY_CHANGE", CountryChange),
    CREATE_COMMAND_BINDING("HEIGHT_CHANGE", HeightChange),
    CREATE_COMMAND_BINDING("CREATE_CREATURE", CreateCreature),
    CREATE_COMMAND_BINDING("CREATE_CREATURE_FROM_FILE", CreateCreatureFromFile),
    CREATE_COMMAND_BINDING("CREATE_FLOCK", CreateFlock),
    CREATE_COMMAND_BINDING("LOAD_LANDSCAPE", LoadLandscape),
    CREATE_COMMAND_BINDING("VERSION", Version),
    CREATE_COMMAND_BINDING("CREATE_AREA", CreateArea),
    CREATE_COMMAND_BINDING("START_CAMERA_POS", StartCameraPos),
    CREATE_COMMAND_BINDING("FLY_BY_FILE", FlyByFile),
    CREATE_COMMAND_BINDING("TOWN_NEEDS_POS", TownNeedsPos),
    CREATE_COMMAND_BINDING("CREATE_FURNITURE", CreateFurniture),
    CREATE_COMMAND_BINDING("CREATE_BIG_FOREST", CreateBigForest),
    CREATE_COMMAND_BINDING("CREATE_NEW_BIG_FOREST", CreateNewBigForest),
    CREATE_COMMAND_BINDING("CREATE_INFLUENCE_RING", CreateInfluenceRing),
    CREATE_COMMAND_BINDING("CREATE_WEATHER_CLIMATE", CreateWeatherClimate),
    CREATE_COMMAND_BINDING("CREATE_WEATHER_CLIMATE_RAIN", CreateWeatherClimateRain),
    CREATE_COMMAND_BINDING("CREATE_WEATHER_CLIMATE_TEMP", CreateWeatherClimateTemp),
    CREATE_COMMAND_BINDING("CREATE_WEATHER_CLIMATE_WIND", CreateWeatherClimateWind),
    CREATE_COMMAND_BINDING("CREATE_WEATHER_STORM", CreateWeatherStorm),
    CREATE_COMMAND_BINDING("BRUSH_SIZE", BrushSize),
    CREATE_COMMAND_BINDING("CREATE_STREAM", CreateStream),
    CREATE_COMMAND_BINDING("CREATE_STREAM_POINT", CreateStreamPoint),
    CREATE_COMMAND_BINDING("CREATE_WATERFALL", CreateWaterfall),
    CREATE_COMMAND_BINDING("CREATE_ARENA", CreateArena),
    CREATE_COMMAND_BINDING("CREATE_FOOTPATH", CreateFootpath),
    CREATE_COMMAND_BINDING("CREATE_FOOTPATH_NODE", CreateFootpathNode),
    CREATE_COMMAND_BINDING("LINK_FOOTPATH", LinkFootpath),
    CREATE_COMMAND_BINDING("CREATE_BONFIRE", CreateBonfire),
    CREATE_COMMAND_BINDING("CREATE_BASE", CreateBase),
    CREATE_COMMAND_BINDING("CREATE_NEW_FEATURE", CreateNewFeature),
    CREATE_COMMAND_BINDING("SET_INTERACT_DESIRE", SetInteractDesire),
    CREATE_COMMAND_BINDING("TOGGLE_COMPUTER_PLAYER", ToggleComputerPlayer),
    CREATE_COMMAND_BINDING("SET_COMPUTER_PLAYER_CREATURE_LIKE", SetComputerPlayerCreatureLike),
    CREATE_COMMAND_BINDING("MULTIPLAYER_DEBUG", MultiplayerDebug),
    CREATE_COMMAND_BINDING("CREATE_STREET_LANTERN", CreateStreetLantern),
    CREATE_COMMAND_BINDING("CREATE_STREET_LIGHT", CreateStreetLight),
    CREATE_COMMAND_BINDING("SET_LAND_NUMBER", SetLandNumber),
    CREATE_COMMAND_BINDING("CREATE_ONE_SHOT_SPELL", CreateOneShotSpell),
    CREATE_COMMAND_BINDING("CREATE_ONE_SHOT_SPELL_PU", CreateOneShotSpellPu),
    CREATE_COMMAND_BINDING("CREATE_FIRE_FLY", CreateFireFly),
    CREATE_COMMAND_BINDING("TOWN_DESIRE_BOOST", TownDesireBoost),
    CREATE_COMMAND_BINDING("CREATE_ANIMATED_STATIC", CreateAnimatedStatic),
    CREATE_COMMAND_BINDING("FIRE_FLY_SPELL_REWARD_PROB", FireFlySpellRewardProb),
    CREATE_COMMAND_BINDING("CREATE_NEW_TOWN_FIELD", CreateNewTownField),
    CREATE_COMMAND_BINDING("CREATE_SPELL_DISPENSER", CreateSpellDispenser),
    CREATE_COMMAND_BINDING("LOAD_COMPUTER_PLAYER_PERSONALLTY", LoadComputerPlayerPersonality),
    CREATE_COMMAND_BINDING("SET_COMPUTER_PLAYER_PERSONALLTY", SetComputerPlayerPersonality),
    CREATE_COMMAND_BINDING("SET_GLOBAL_LAND_BALANCE", SetGlobalLandBalance),
    CREATE_COMMAND_BINDING("SET_LAND_BALANCE", SetLandBalance),
    CREATE_COMMAND_BINDING("CREATE_DRINK_WAYPOINT", CreateDrinkWaypoint),
    CREATE_COMMAND_BINDING("SET_TOWN_INFLUENCE_MULTIPLIER", SetTownInfluenceMultiplier),
    CREATE_COMMAND_BINDING("SET_PLAYER_INFLUENCE_MULTIPLIER", SetPlayerInfluenceMultiplier),
    CREATE_COMMAND_BINDING("SET_TOWN_BALANCE_BELIEF_SCALE", SetTownBalanceBeliefScale),
    CREATE_COMMAND_BINDING("START_GAME_MESSAGE", StartGameMessage),
    CREATE_COMMAND_BINDING("ADD_GAME_MESSAGE_LINE", AddGameMessageLine),
    CREATE_COMMAND_BINDING("EDIT_LEVEL", EditLevel),
    CREATE_COMMAND_BINDING("SET_NIGHTTIME", SetNighttime),
    CREATE_COMMAND_BINDING("MAKE_LAST_OBJECT_ARTIFACT", MakeLastObjectArtifact),
    CREATE_COMMAND_BINDING("SET_LOST_TOWN_SCALE", SetLostTownScale),
}};

Feature::Info GetFeatureInfo(const std::string& featureType)
{
	auto item = featureInfoLookup.find(featureType);

	if (item == featureInfoLookup.end())
	{
		SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "Missing abode mesh lookup for \"{}\".", featureType);
		return Feature::Info::Ark;
	}

	return item->second;
}

inline glm::mat4 GetRotation(int rotation)
{
	return glm::eulerAngleY(static_cast<float>(rotation) * -0.001f);
}

inline glm::vec3 GetSize(int size)
{
	return glm::vec3(size, size, size) * 0.001f;
}

void FeatureScriptCommands::CreateMist(glm::vec3 position, float param_2, int32_t param_3, float param_4, float param_5)
{
	SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	                    __func__);
}

void FeatureScriptCommands::CreatePath(int32_t param_1, int32_t param_2, int32_t param_3, int32_t param_4)
{
	SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	                    __func__);
}

void FeatureScriptCommands::CreateTown(int32_t townId, glm::vec3 position, const std::string& playerOwner,
                                       [[maybe_unused]] int32_t, const std::string& tribeStr)
{
	SPDLOG_LOGGER_DEBUG(spdlog::get("scripting"), R"(LHScriptX: Creating town {} for "{}" with tribe "{}".)", townId,
	                    playerOwner, tribeStr);

	auto& registry = Game::instance()->GetEntityRegistry();
	const auto entity = registry.Create();

	registry.Assign<Town>(entity, townId, tribeLookup.at(tribeStr));
	auto& registryContext = registry.Context();
	registryContext.towns.insert({townId, entity});
}

void FeatureScriptCommands::SetTownBelief(int32_t townId, const std::string& playerOwner, float belief)
{
	auto& registry = Game::instance()->GetEntityRegistry();
	auto& registryContext = registry.Context();

	Town& town = registry.Get<Town>(registryContext.towns.at(townId));
	town.beliefs.insert({playerOwner, belief});
}

void FeatureScriptCommands::SetTownBeliefCap(int32_t townId, const std::string& playerOwner, float belief)
{
	SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	                    __func__);
}

void FeatureScriptCommands::SetTownUninhabitable(int32_t townId)
{
	SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	                    __func__);
}

void FeatureScriptCommands::SetTownCongregationPos(int32_t townId, glm::vec3 position)
{
	SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	                    __func__);
}

void FeatureScriptCommands::CreateAbode(int32_t townId, glm::vec3 position, const std::string& abodeInfo, int32_t rotation,
                                        int32_t size, int32_t foodAmount, int32_t woodAmount)
{
	Abode::Create(static_cast<uint32_t>(townId), position, abodeInfo, GetRotation(rotation), GetSize(size),
	              static_cast<uint32_t>(foodAmount), static_cast<uint32_t>(woodAmount), false);
}

void FeatureScriptCommands::CreatePlannedAbode(int32_t townId, glm::vec3 position, const std::string& abodeInfo,
                                               int32_t rotation, int32_t size, int32_t foodAmount, int32_t woodAmount)
{
	Abode::Create(static_cast<uint32_t>(townId), position, abodeInfo, GetRotation(rotation), GetSize(size),
	              static_cast<uint32_t>(foodAmount), static_cast<uint32_t>(woodAmount), true);
}

void FeatureScriptCommands::CreateTownCentre(int32_t townId, glm::vec3 position, const std::string& abodeInfo, int32_t rotation,
                                             int32_t size, int32_t)
{
	auto& registry = Game::instance()->GetEntityRegistry();
	const auto entity = registry.Create();
	auto submeshIds = std::vector {3};

	registry.Assign<Transform>(entity, position, GetRotation(rotation), GetSize(size));
	registry.Assign<Fixed>(entity);
	const auto& abode = registry.Assign<Abode>(entity, Abode::GetInfo(abodeInfo).value_or(Abode::Info::TibetanWonder),
	                                           static_cast<int>(townId), static_cast<uint32_t>(0), static_cast<uint32_t>(0));
	registry.Assign<Mesh>(entity, abodeMeshLookup[abode.type], static_cast<int8_t>(0), static_cast<int8_t>(0));
}

void FeatureScriptCommands::CreateTownSpell(int32_t townId, const std::string& spellName)
{
	SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	                    __func__);
}

void FeatureScriptCommands::CreateNewTownSpell(int32_t townId, const std::string& spellName)
{
	SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	                    __func__);
}

void FeatureScriptCommands::CreateTownCentreSpellIcon(int32_t param_1, const std::string& param_2)
{
	SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	                    __func__);
}

void FeatureScriptCommands::CreateSpellIcon(glm::vec3 position, const std::string& param_2, int32_t param_3, int32_t param_4,
                                            int32_t param_5)
{
	SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	                    __func__);
}

void FeatureScriptCommands::CreatePlannedSpellIcon(int32_t param_1, glm::vec3 position, const std::string& param_3,
                                                   int32_t param_4, int32_t param_5, int32_t param_6)
{
	SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	                    __func__);
}

void FeatureScriptCommands::CreateVillager(glm::vec3 position, const std::string& tribeAndRole, int32_t age)
{
	CreateVillagerPos(position, position, tribeAndRole, age);
}

void FeatureScriptCommands::CreateTownVillager(int32_t townId, glm::vec3 position, const std::string& villagerType, int32_t age)
{
	SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	                    __func__);
}

void FeatureScriptCommands::CreateSpecialTownVillager(int32_t, glm::vec3, int32_t, int32_t)
{
	SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	                    __func__);
}

void FeatureScriptCommands::CreateVillagerPos(glm::vec3 abodePosition, glm::vec3 position, const std::string& tribeAndRole,
                                              int32_t age)
{
	const auto [tribe, role] = GetVillagerTribeAndRole(tribeAndRole);
	Villager::Create(abodePosition, position, tribe, role, static_cast<uint32_t>(age));
}

void FeatureScriptCommands::CreateCitadel(glm::vec3 position, int32_t, const std::string&, int32_t, int32_t)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::CreatePlannedCitadel(int32_t, glm::vec3 position, int32_t, const std::string&, int32_t, int32_t)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::CreateCreaturePen(glm::vec3 position, int32_t, int32_t, int32_t, int32_t, int32_t)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::CreateWorshipSite(glm::vec3 position, int32_t, const std::string&, const std::string&, int32_t,
                                              int32_t)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::CreatePlannedWorshipSite(glm::vec3 position, int32_t, const std::string&, const std::string&,
                                                     int32_t, int32_t)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::CreateAnimal(glm::vec3 position, int32_t, int32_t, int32_t)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::CreateNewAnimal(glm::vec3 position, int32_t, int32_t, int32_t, int32_t)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::CreateForest(int32_t forestId, glm::vec3 position)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::CreateTree(int32_t forestId, glm::vec3 position, int32_t, int32_t, int32_t)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::CreateNewTree(int32_t forestId, glm::vec3 position, int32_t treeType, int32_t isNonScenic,
                                          float rotation, float currentSize, float maxSize)
{
	auto& registry = Game::instance()->GetEntityRegistry();
	const auto entity = registry.Create();

	registry.Assign<Transform>(entity, position, glm::eulerAngleY(-rotation), glm::vec3(currentSize));
	registry.Assign<Fixed>(entity);
	const auto& tree = registry.Assign<Tree>(entity, Tree::Info(treeType));
	registry.Assign<Mesh>(entity, treeMeshLookup[tree.type], static_cast<int8_t>(0), static_cast<int8_t>(-1));
}

void FeatureScriptCommands::CreateField(glm::vec3 position, int32_t)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::CreateTownField(int32_t townId, glm::vec3 position, int32_t)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::CreateFishFarm(glm::vec3 position, int32_t)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::CreateTownFishFarm(int32_t townId, glm::vec3 position, int32_t)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::CreateFeature(glm::vec3 position, int32_t, int32_t, int32_t, int32_t)
{
	SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	                    __func__);
}

void FeatureScriptCommands::CreateFlowers(glm::vec3 position, int32_t, float, float)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::CreateWallSection(glm::vec3 position, int32_t, int32_t, int32_t, int32_t)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::CreatePlannedWallSection(glm::vec3 position, int32_t, int32_t, int32_t, int32_t)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::CreatePitch(glm::vec3 position, int32_t, int32_t, int32_t, int32_t, int32_t)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::CreatePot(glm::vec3 position, int32_t, int32_t, int32_t)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::CreateTownTemporaryPots(int32_t, int32_t, int32_t)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::CreateMobileObject(glm::vec3 position, int32_t type, int32_t rotation, int32_t scale)
{
	auto& registry = Game::instance()->GetEntityRegistry();
	const auto entity = registry.Create();

	registry.Assign<Transform>(entity, position, GetRotation(rotation), GetSize(scale));
	registry.Assign<Mobile>(entity);
	const auto& object = registry.Assign<MobileObject>(entity, static_cast<MobileObject::Info>(type));
	registry.Assign<Mesh>(entity, mobileObjectMeshLookup[object.type], static_cast<int8_t>(0), static_cast<int8_t>(1));
}

void FeatureScriptCommands::CreateMobileStatic(glm::vec3 position, int32_t, float, float)
{
	SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	                    __func__);
}

void FeatureScriptCommands::CreateMobileUStatic(glm::vec3 position, int32_t type, float verticalOffset, float pitch,
                                                float rotation, float lean, float scale)
{
	auto& registry = Game::instance()->GetEntityRegistry();
	const auto entity = registry.Create();

	glm::vec3 offset(0.0f, verticalOffset, 0.0f);

	registry.Assign<Transform>(entity, position + offset, glm::eulerAngleXYZ(-pitch, -rotation, -lean), glm::vec3(scale));
	registry.Assign<Mobile>(entity);
	const auto& mobile = registry.Assign<MobileStatic>(entity, MobileStatic::Info(type));
	registry.Assign<Mesh>(entity, mobileStaticMeshLookup[mobile.type], static_cast<int8_t>(0), static_cast<int8_t>(1));
}

void FeatureScriptCommands::CreateDeadTree(glm::vec3 position, const std::string&, int32_t, float, float, float, float)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::CreateScaffold(int32_t, glm::vec3 position, int32_t, int32_t, int32_t)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::CountryChange(glm::vec3 position, int32_t)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::HeightChange(glm::vec3 position, int32_t)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::CreateCreature(glm::vec3 position, int32_t, int32_t)
{
	SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	                    __func__);
}

void FeatureScriptCommands::CreateCreatureFromFile(const std::string& playerName, int32_t creatureType,
                                                   const std::string& creatureMind, glm::vec3 position)
{
	SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	                    __func__);
}

void FeatureScriptCommands::CreateFlock(int32_t, glm::vec3, glm::vec3, int32_t, int32_t, int32_t)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::LoadLandscape(const std::string& path)
{
	Game::instance()->LoadLandscape(path);
}

void FeatureScriptCommands::Version(float version)
{
	SPDLOG_LOGGER_DEBUG(spdlog::get("scripting"), "LHScriptX: Land version set to: {}", version);
}

void FeatureScriptCommands::CreateArea(glm::vec3 position, float)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::StartCameraPos(glm::vec3 position)
{
	auto& camera = Game::instance()->GetCamera();
	const glm::vec3 offset(0.0f, 10.0f, 0.0f);
	camera.SetPosition(position + offset);
}

void FeatureScriptCommands::FlyByFile(const std::string& path)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::TownNeedsPos(int32_t townId, glm::vec3 position)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::CreateFurniture(glm::vec3 position, int32_t, float)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::CreateBigForest(glm::vec3 position, int32_t, float, float)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::CreateNewBigForest(glm::vec3 position, int32_t type, int32_t param_3, float rotation, float scale)
{
	auto& registry = Game::instance()->GetEntityRegistry();
	const auto entity = registry.Create();

	registry.Assign<Transform>(entity, position, glm::eulerAngleY(-rotation), glm::vec3(scale));
	registry.Assign<Fixed>(entity);
	registry.Assign<Forest>(entity);
	registry.Assign<Mesh>(entity, MeshPackId::FeatureForest, static_cast<int8_t>(0), static_cast<int8_t>(1));
}

void FeatureScriptCommands::CreateInfluenceRing(glm::vec3 position, int32_t, float, int32_t)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::CreateWeatherClimate(int32_t, int32_t, glm::vec3, float, float)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::CreateWeatherClimateRain(int32_t, float, int32_t, int32_t, int32_t)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::CreateWeatherClimateTemp(int32_t, float, float)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::CreateWeatherClimateWind(int32_t, float, float, float)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::CreateWeatherStorm(int32_t, glm::vec3, float, int32_t, glm::vec3, glm::vec3, glm::vec3, float,
                                               glm::vec3)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::BrushSize(float, float)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::CreateStream(int32_t streamId)
{
	auto& registry = Game::instance()->GetEntityRegistry();
	auto& registryContext = registry.Context();
	const auto entity = registry.Create();

	registry.Assign<Stream>(entity, streamId);
	registryContext.streams.insert({streamId, entity});
}

void FeatureScriptCommands::CreateStreamPoint(int32_t streamId, glm::vec3 position)
{
	auto& registry = Game::instance()->GetEntityRegistry();
	auto& registryContext = registry.Context();

	Stream& stream = registry.Get<Stream>(registryContext.streams.at(streamId));
	stream.nodes.emplace_back(position, stream.nodes);
}

void FeatureScriptCommands::CreateWaterfall(glm::vec3 position)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::CreateArena(glm::vec3 position, float)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::CreateFootpath(int32_t footpathId)
{
	auto& registry = Game::instance()->GetEntityRegistry();
	const auto entity = registry.Create();
	registry.Assign<Footpath>(entity);
	auto& registryContext = registry.Context();
	registryContext.footpaths.insert({footpathId, entity});
}

void FeatureScriptCommands::CreateFootpathNode(int footpathId, glm::vec3 position)
{
	auto& registry = Game::instance()->GetEntityRegistry();
	auto& registryContext = registry.Context();
	auto& footpath = registry.Get<Footpath>(registryContext.footpaths.at(footpathId));
	footpath.nodes.emplace_back(Footpath::Node {position});
}

void FeatureScriptCommands::LinkFootpath(int32_t footpathId)
{
	// TODO: The last MultiMapFixed created in this script is an implicit param
	//       This Command adds the footpath to a list in a FootpathLink on the MultiMapFixed
	SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	                    __func__);
}

void FeatureScriptCommands::CreateBonfire(glm::vec3 position, float rotation, float param_3, float scale)
{
	auto& registry = Game::instance()->GetEntityRegistry();
	const auto entity = registry.Create();

	registry.Assign<Transform>(entity, position, glm::eulerAngleY(-rotation), glm::vec3(scale));
	registry.Assign<Mobile>(entity);
	const auto& mobile = registry.Assign<MobileStatic>(entity, MobileStatic::Info::Bonfire);
	registry.Assign<Mesh>(entity, mobileStaticMeshLookup[mobile.type], static_cast<int8_t>(0), static_cast<int8_t>(1));
}

void FeatureScriptCommands::CreateBase(glm::vec3 position, int32_t)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::CreateNewFeature(glm::vec3 position, const std::string& type, int32_t rotation, int32_t scale,
                                             int32_t param_5)
{
	auto& registry = Game::instance()->GetEntityRegistry();
	const auto entity = registry.Create();

	registry.Assign<Transform>(entity, position, GetRotation(rotation), GetSize(scale));
	registry.Assign<Fixed>(entity);
	const auto& feature = registry.Assign<Feature>(entity, GetFeatureInfo(type));
	registry.Assign<Mesh>(entity, featureMeshLookup[feature.type], static_cast<int8_t>(0), static_cast<int8_t>(1));
}

void FeatureScriptCommands::SetInteractDesire(float)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::ToggleComputerPlayer(const std::string&, int32_t)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::SetComputerPlayerCreatureLike(const std::string&, const std::string&)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::MultiplayerDebug(int32_t, int32_t)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::CreateStreetLantern(glm::vec3 position, int32_t)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::CreateStreetLight(glm::vec3 position)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::SetLandNumber(int32_t number)
{
	SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	                    __func__);
}

void FeatureScriptCommands::CreateOneShotSpell(glm::vec3 position, const std::string&)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::CreateOneShotSpellPu(glm::vec3 position, const std::string&)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::CreateFireFly(glm::vec3 position)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::TownDesireBoost(int32_t townId, const std::string&, float)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::CreateAnimatedStatic(glm::vec3 position, const std::string& type, int32_t rotation, int32_t scale)
{
	auto& registry = Game::instance()->GetEntityRegistry();
	const auto entity = registry.Create();

	registry.Assign<Transform>(entity, position, GetRotation(rotation), GetSize(scale));
	registry.Assign<Fixed>(entity);
	const auto& animated = registry.Assign<AnimatedStatic>(entity, type);
	MeshPackId meshPackId = MeshPackId::Dummy;
	if (animated.type == "Norse Gate")
	{
		meshPackId = MeshPackId::BuildingNorseGate;
	}
	else if (animated.type == "Gate Stone Plinth")
	{
		meshPackId = MeshPackId::ObjectGateTotemPlinthe;
	}
	else if (animated.type == "Piper Cave Entrance")
	{
		meshPackId = MeshPackId::BuildingMineEntrance;
	}
	registry.Assign<Mesh>(entity, static_cast<MeshId>(meshPackId), static_cast<int8_t>(0), static_cast<int8_t>(0));
}

void FeatureScriptCommands::FireFlySpellRewardProb(const std::string& spell, float probability)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::CreateNewTownField(int32_t townId, glm::vec3 position, int32_t param_3, float rotation)
{
	auto& registry = Game::instance()->GetEntityRegistry();
	const auto entity = registry.Create();

	registry.Assign<Transform>(entity, position, GetRotation(rotation), GetSize(1000));
	registry.Assign<Fixed>(entity);
	registry.Assign<Field>(entity, townId);
	registry.Assign<Mesh>(entity, MeshPackId::TreeWheat, static_cast<int8_t>(0), static_cast<int8_t>(0));
}

void FeatureScriptCommands::CreateSpellDispenser(int32_t, glm::vec3 position, const std::string&, const std::string&, float,
                                                 float, float)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::LoadComputerPlayerPersonality(int32_t, glm::vec3)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::SetComputerPlayerPersonality(const std::string&, glm::vec3, float)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::SetGlobalLandBalance(int32_t, float)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::SetLandBalance(const std::string&, int32_t, float)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::CreateDrinkWaypoint(glm::vec3 position)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::SetTownInfluenceMultiplier(float multiplier)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::SetPlayerInfluenceMultiplier(float multiplier)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::SetTownBalanceBeliefScale(int32_t townId, float scale)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::StartGameMessage(const std::string& message, int32_t landNumber)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::AddGameMessageLine(const std::string& message, int32_t landNumber)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::EditLevel()
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::SetNighttime(float, float, float)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::MakeLastObjectArtifact(int32_t, const std::string&, float)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}

void FeatureScriptCommands::SetLostTownScale(float scale)
{
	// SPDLOG_LOGGER_ERROR(spdlog::get("scripting"), "LHScriptX: {}:{}: Function {} not implemented.", __FILE__, __LINE__,
	// __func__);
}
