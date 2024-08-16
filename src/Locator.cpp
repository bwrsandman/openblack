/******************************************************************************
 * Copyright (c) 2018-2024 openblack developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/openblack/openblack
 *
 * openblack is licensed under the GNU General Public License version 3.
 *******************************************************************************/

#include "Locator.h"

#define LOCATOR_IMPLEMENTATIONS

#include <spdlog/spdlog.h>

#include "3D/LandIsland.h"
#include "3D/TempleInterior.h"
#include "3D/UnloadedIsland.h"
#include "Audio/AudioManager.h"
#include "Audio/AudioManagerNoOp.h"
#include "Common/EventManager.h"
#include "Common/RandomNumberManagerProduction.h"
#include "Debug/DebugGuiInterface.h"
#include "ECS/Archetypes/PlayerArchetype.h"
#include "ECS/MapProduction.h"
#include "ECS/Registry.h"
#include "ECS/Systems/Implementations/CameraBookmarkSystem.h"
#include "ECS/Systems/Implementations/DynamicsSystem.h"
#include "ECS/Systems/Implementations/LivingActionSystem.h"
#include "ECS/Systems/Implementations/PathfindingSystem.h"
#include "ECS/Systems/Implementations/PlayerSystem.h"
#include "ECS/Systems/Implementations/RenderingSystem.h"
#include "ECS/Systems/Implementations/TownSystem.h"
#include "Graphics/RendererInterface.h"
#include "Input/GameActionMap.h"
#include "Profiler.h"
#include "Resources/Resources.h"
#include "Windowing/Sdl2WindowingSystem.h"
#if __ANDROID__
#include "FileSystem/AndroidFileSystem.h"
#else
#include "FileSystem/DefaultFileSystem.h"
#endif

using namespace openblack::audio;
using namespace openblack::filesystem;
using openblack::LandIsland;
using openblack::RandomNumberManagerProduction;
using openblack::TempleInterior;
using openblack::UnloadedIsland;
using openblack::debug::gui::DebugGuiInterface;
using openblack::ecs::MapProduction;
using openblack::ecs::Registry;
using openblack::ecs::systems::CameraBookmarkSystem;
using openblack::ecs::systems::DynamicsSystem;
using openblack::ecs::systems::LivingActionSystem;
using openblack::ecs::systems::PathfindingSystem;
using openblack::ecs::systems::PlayerSystem;
using openblack::ecs::systems::RenderingSystem;
using openblack::ecs::systems::TownSystem;
using openblack::graphics::RendererInterface;
using openblack::input::GameActionMap;
using openblack::resources::Resources;
using openblack::windowing::DisplayMode;
using openblack::windowing::Sdl2WindowingSystem;

void openblack::InitializeWindow(const std::string& title, int width, int height, DisplayMode displayMode, uint32_t extraFlags)
{
	Locator::windowing::emplace<Sdl2WindowingSystem>(title, width, height, displayMode, extraFlags);
}

bool openblack::InitializeGame(uint8_t rendererType, bool vsync) noexcept
{
	SPDLOG_LOGGER_INFO(spdlog::get("game"), "EnTT version: {}", ENTT_VERSION);
	SPDLOG_LOGGER_INFO(spdlog::get("game"), GLM_VERSION_MESSAGE);

	Locator::profiler::emplace();

	Locator::rendererInterface::reset(
	    RendererInterface::Create(static_cast<bgfx::RendererType::Enum>(rendererType), vsync).release());
	if (!Locator::rendererInterface::has_value())
	{
		SPDLOG_LOGGER_CRITICAL(spdlog::get("graphics"), "Failed to create renderer");
		return false;
	}
	Locator::debugGui::reset(DebugGuiInterface::Create(graphics::RenderPass::ImGui).release());
	Locator::events::emplace<EventManager>();

#if __ANDROID__
	Locator::filesystem::emplace<AndroidFileSystem>();
#else
	Locator::filesystem::emplace<DefaultFileSystem>();
#endif
	Locator::terrainSystem::emplace<UnloadedIsland>();
	Locator::resources::emplace<Resources>();
	Locator::rng::emplace<RandomNumberManagerProduction>();
	try
	{
		Locator::audio::emplace<AudioManager>();
	}
	catch (std::runtime_error& error)
	{
		SPDLOG_LOGGER_ERROR(spdlog::get("audio"), "Falling back to no-op audio: {}", error.what());
		Locator::audio::emplace<AudioManagerNoOp>();
	}
	Locator::playerSystem::emplace<PlayerSystem>();
	Locator::gameActionSystem::emplace<GameActionMap>();
	Locator::rendereringSystem::emplace<RenderingSystem>();
	Locator::entitiesRegistry::emplace<Registry>();
	Locator::temple::emplace<TempleInterior>();

	return true;
}

void openblack::InitializeLevel(const std::filesystem::path& path)
{
	Locator::entitiesMap::emplace<MapProduction>();
	Locator::dynamicsSystem::emplace<DynamicsSystem>();
	Locator::livingActionSystem::emplace<LivingActionSystem>();
	Locator::townSystem::emplace<TownSystem>();
	Locator::pathfindingSystem::emplace<PathfindingSystem>();
	Locator::cameraBookmarkSystem::emplace<CameraBookmarkSystem>();
	Locator::terrainSystem::emplace<LandIsland>(path);
}
