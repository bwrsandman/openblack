/*****************************************************************************
 * Copyright (c) 2018-2022 openblack developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/openblack/openblack
 *
 * openblack is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <tuple>

#include "json.hpp"
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/vec_swizzle.hpp>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <ECS/Components/Transform.h>
#include <ECS/Components/Villager.h>
#include <ECS/Components/WallHug.h>
#include <ECS/Map.h>
#include <ECS/Registry.h>
#include <ECS/Systems/PathfindingSystem.h>
#include <Game.h>
#include <LHScriptX/Script.h>

using nlohmann::json;
using namespace openblack;

namespace glm
{
template <length_t L, typename T, qualifier Q>
std::ostream& operator<<(std::ostream& os, const vec<L, T, Q>& v)
{
	return os << glm::to_string(v);
}
} // namespace glm

enum MOVE_STATE
{
	MOVE_STATE_ARRIVED,
	MOVE_STATE_FINAL_STEP,
	MOVE_STATE_STEP_THROUGH,
	MOVE_STATE_LINEAR,
	MOVE_STATE_LINEAR_CW,
	MOVE_STATE_LINEAR_CCW,
	MOVE_STATE_ORBIT_CW,
	MOVE_STATE_ORBIT_CCW,
	MOVE_STATE_EXIT_CIRCLE_CCW,
	MOVE_STATE_EXIT_CIRCLE_CW,

	_MOVE_STATE_INVALID = -1,
};

NLOHMANN_JSON_SERIALIZE_ENUM(MOVE_STATE, {
                                             {_MOVE_STATE_INVALID, nullptr},
                                             {MOVE_STATE_ARRIVED, "ARRIVED"},
                                             {MOVE_STATE_FINAL_STEP, "FINAL_STEP"},
                                             {MOVE_STATE_STEP_THROUGH, "STEP_THROUGH"},
                                             {MOVE_STATE_LINEAR, "LINEAR"},
                                             {MOVE_STATE_LINEAR_CW, "LINEAR_CW"},
                                             {MOVE_STATE_LINEAR_CCW, "LINEAR_CCW"},
                                             {MOVE_STATE_ORBIT_CW, "ORBIT_CW"},
                                             {MOVE_STATE_ORBIT_CCW, "ORBIT_CCW"},
                                             {MOVE_STATE_EXIT_CIRCLE_CCW, "EXIT_CIRCLE_CCW"},
                                             {MOVE_STATE_EXIT_CIRCLE_CW, "EXIT_CIRCLE_CW"},
                                         })

enum VILLAGER_STATE
{
	VILLAGER_STATE_MOVE_TO_POS,
	VILLAGER_STATE_GO_AND_CHILLOUT_OUTSIDE_HOME,
	VILLAGER_STATE_MOVE_ON_PATH,
	VILLAGER_STATE_ARRIVES_HOME,

	_VILLAGER_STATE_INVALID = -1,
};

NLOHMANN_JSON_SERIALIZE_ENUM(VILLAGER_STATE, {
                                                 {_VILLAGER_STATE_INVALID, nullptr},
                                                 {VILLAGER_STATE_MOVE_TO_POS, "MOVE_TO_POS"},
                                                 {VILLAGER_STATE_GO_AND_CHILLOUT_OUTSIDE_HOME, "GO_AND_CHILLOUT_OUTSIDE_HOME"},
                                                 {VILLAGER_STATE_MOVE_ON_PATH, "MOVE_ON_PATH"},
                                                 {VILLAGER_STATE_ARRIVES_HOME, "ARRIVES_HOME"},
                                             })

class MobileWallHugWalks: public ::testing::Test
{
protected:
	struct State
	{
		struct CircleHugInfo
		{
			std::optional<uint32_t> obj_index;
			uint8_t turns_to_obstacle;
			uint8_t field_0x5;  // TODO(bwrsandman): Unknown function
			uint16_t field_0x6; // TODO(bwrsandman): Unknown function
		};

		uint32_t turn;
		uint32_t id;
		glm::vec2 pos;
		uint16_t field_0x24; // TODO(bwrsandman): Unknown function
		uint8_t field_0x26;  // TODO(bwrsandman): Unknown function
		uint32_t turns_until_next_state_change;
		float speed;
		float y_angle;
		MOVE_STATE move_state;
		glm::vec2 step;
		CircleHugInfo circle_hug_info;
		uint32_t status; // Probably unused
		glm::vec2 goal;
		float distance; // Derived
		std::tuple<VILLAGER_STATE, VILLAGER_STATE> state_map;
	};

	void SetUp() override
	{
		const auto test_name = ::testing::UnitTest::GetInstance()->current_test_info()->name();
		const auto test_results_path = std::filesystem::path(scenario_path) / (test_name + std::string(".json"));
		json results;
		std::ifstream(test_results_path) >> results;
		start_turn = results["start_turn"];
		last_turn = results["last_turn"];
		const auto test_scene_path = std::filesystem::path(scenario_path) / results["map_file"];
		const auto& villager_states = results["villager_states"];
		expected_states.reserve(villager_states.size());
#define GET_SAME_TYPE_AS(name) s[#name].get<decltype(expected_states[0].name)>()
#define GET_SAME_TYPE_AS_HEX(name) \
	static_cast<decltype(expected_states[0].name)>(std::stoul(s[#name].get<std::string>(), nullptr, 16))
#define GET_VEC2(name) decltype(expected_states[0].name)(s[#name][0].get<float>(), s[#name][1].get<float>())
		for (const auto& s : villager_states)
		{
			auto circle_hug_info = s["circle_hug_info"];
			const auto& e = expected_states.emplace_back(State {
			    GET_SAME_TYPE_AS(turn),
			    GET_SAME_TYPE_AS(id),
			    GET_VEC2(pos),
			    GET_SAME_TYPE_AS_HEX(field_0x24),
			    GET_SAME_TYPE_AS_HEX(field_0x26),
			    GET_SAME_TYPE_AS(turns_until_next_state_change),
			    GET_SAME_TYPE_AS(speed),
			    GET_SAME_TYPE_AS(y_angle),
			    GET_SAME_TYPE_AS(move_state),
			    GET_VEC2(step),
			    {
			        circle_hug_info["obj_index"].is_null() ? std::nullopt
			                                               : std::make_optional(circle_hug_info["obj_index"].get<uint32_t>()),
			        static_cast<uint8_t>(std::stoi(circle_hug_info["turns_to_obj"].get<std::string>(), nullptr, 16)),
			        static_cast<uint8_t>(std::stoi(circle_hug_info["field_0x5"].get<std::string>(), nullptr, 16)),
			        static_cast<uint16_t>(std::stoi(circle_hug_info["field_0x6"].get<std::string>(), nullptr, 16)),
			    },
			    GET_SAME_TYPE_AS_HEX(status),
			    GET_VEC2(goal),
			    GET_SAME_TYPE_AS(distance),
			    GET_SAME_TYPE_AS(state_map),
			});

			ASSERT_NE(e.move_state, _MOVE_STATE_INVALID) << "Unexpected Move State: " + s["move_state"].get<std::string>();
			ASSERT_NE(std::get<0>(e.state_map), _VILLAGER_STATE_INVALID)
			    << "Unexpected Villager State: " + s["state_map"][0].get<std::string>();
			ASSERT_NE(std::get<1>(e.state_map), _VILLAGER_STATE_INVALID)
			    << "Unexpected Villager State: " + s["state_map"][1].get<std::string>();
		}

#undef GET_SAME_TYPE_AS
#undef GET_SAME_TYPE_AS_HEX
#undef GET_MAP_COORDS

		ASSERT_TRUE(std::filesystem::exists(test_scene_path));

		{
			std::ifstream ifs(test_scene_path);
			scene_script = std::string(std::istreambuf_iterator<char> {ifs}, {});
		}

		static const auto mock_game_path = std::filesystem::path(TEST_BINARY_DIR) / "mock";
		auto args = openblack::Arguments {
		    .rendererType = bgfx::RendererType::Enum::Noop,
		    .gamePath = mock_game_path.string(),
		    .logFile = "stdout",
		};
		std::fill_n(args.logLevels.begin(), args.logLevels.size(), spdlog::level::warn);
		args.logLevels[static_cast<uint8_t>(openblack::LoggingSubsystem::pathfinding)] = spdlog::level::debug;
		game = std::make_unique<openblack::Game>(std::move(args));
		ASSERT_TRUE(game->Initialize());
		auto script = openblack::lhscriptx::Script(game.get());
		script.Load(scene_script);

		villager_entt = game->GetEntityRegistry().Front<const ecs::components::Villager>();
		auto& villager_transform = game->GetEntityRegistry().Get<ecs::components::Transform>(villager_entt);

		villager_transform.position = glm::vec3(expected_states[0].pos.x, 0.0f, expected_states[0].pos.y);
	}

	void TearDown() override { game.reset(); }

	void MobileWallHugScenarioAssert()
	{
		auto& map = Game::instance()->GetEntityMap();
		auto& registry = Game::instance()->GetEntityRegistry();
		map.Rebuild();
		registry.Each<ecs::components::WallHug>([&registry, this](entt::entity entity, ecs::components::WallHug& wallHug) {
			using namespace openblack::ecs::components;
			registry.Assign<MoveStateLinearTag>(entity);
			wallHug.speed = expected_states[0].speed;
			wallHug.step = expected_states[0].step;
			wallHug.goal = expected_states[0].goal;
		});

		for (uint32_t turn = start_turn; turn < last_turn; ++turn)
		{
			const auto& villager_comp = registry.Get<ecs::components::Villager>(villager_entt);
			const auto& villager_transform = registry.Get<ecs::components::Transform>(villager_entt);
			const auto& villager_wallhug = registry.Get<ecs::components::WallHug>(villager_entt);
			bool villager_has_obstacle = registry.AnyOf<ecs::components::WallHugObjectReference>(villager_entt);
			const auto& state = expected_states[turn - start_turn];
			const auto msg = std::string("on turn ") + std::to_string(turn) + " in range " + std::to_string(start_turn) + "-" +
			                 std::to_string(last_turn);

			switch (state.move_state)
			{
			default:
			case _MOVE_STATE_INVALID:
				ASSERT_TRUE(false);
				break;
			case MOVE_STATE_LINEAR:
			case MOVE_STATE_LINEAR_CW:
			case MOVE_STATE_LINEAR_CCW:
			{
				ASSERT_TRUE(registry.AllOf<ecs::components::MoveStateLinearTag>(villager_entt)) << msg;
				const auto& villager_state = registry.Get<ecs::components::MoveStateLinearTag>(villager_entt);
				if (state.move_state == MOVE_STATE_LINEAR)
					ASSERT_EQ(villager_state.clockwise, ecs::components::MoveStateClockwise::Undefined) << msg;
				else if (state.move_state == MOVE_STATE_LINEAR_CW)
					ASSERT_EQ(villager_state.clockwise, ecs::components::MoveStateClockwise::Clockwise) << msg;
				else if (state.move_state == MOVE_STATE_LINEAR_CCW)
					ASSERT_EQ(villager_state.clockwise, ecs::components::MoveStateClockwise::CounterClockwise) << msg;
			}
			break;
			case MOVE_STATE_ORBIT_CW:
			case MOVE_STATE_ORBIT_CCW:
			{
				ASSERT_TRUE(registry.AllOf<ecs::components::MoveStateOrbitTag>(villager_entt)) << msg;
				const auto& villager_state = registry.Get<ecs::components::MoveStateOrbitTag>(villager_entt);
				if (state.move_state == MOVE_STATE_ORBIT_CW)
					ASSERT_EQ(villager_state.clockwise, ecs::components::MoveStateClockwise::Clockwise) << msg;
				else if (state.move_state == MOVE_STATE_ORBIT_CCW)
					ASSERT_EQ(villager_state.clockwise, ecs::components::MoveStateClockwise::CounterClockwise) << msg;
			}
			break;
			case MOVE_STATE_EXIT_CIRCLE_CW:
			case MOVE_STATE_EXIT_CIRCLE_CCW:
			{
				ASSERT_TRUE(registry.AllOf<ecs::components::MoveStateExitCircleTag>(villager_entt)) << msg;
				const auto& villager_state = registry.Get<ecs::components::MoveStateExitCircleTag>(villager_entt);
				if (state.move_state == MOVE_STATE_EXIT_CIRCLE_CW)
					ASSERT_EQ(villager_state.clockwise, ecs::components::MoveStateClockwise::Clockwise) << msg;
				else if (state.move_state == MOVE_STATE_EXIT_CIRCLE_CCW)
					ASSERT_EQ(villager_state.clockwise, ecs::components::MoveStateClockwise::CounterClockwise) << msg;
			}
			break;
			case MOVE_STATE_ARRIVED:
				ASSERT_TRUE(registry.AllOf<ecs::components::MoveStateArrivedTag>(villager_entt)) << msg;
				break;
			case MOVE_STATE_FINAL_STEP:
				ASSERT_TRUE(registry.AllOf<ecs::components::MoveStateFinalStepTag>(villager_entt)) << msg;
				break;
			case MOVE_STATE_STEP_THROUGH:
				ASSERT_TRUE(registry.AllOf<ecs::components::MoveStateStepThroughTag>(villager_entt)) << msg;
				break;
			}
			ASSERT_EQ(glm::xz(villager_transform.position), state.pos) << msg;
			if (state.move_state != MOVE_STATE_FINAL_STEP) // Set in the next turn
			{
				ASSERT_EQ(villager_wallhug.step, state.step) << msg;
			}
			ASSERT_EQ(villager_wallhug.goal, state.goal) << msg;
			ASSERT_EQ(villager_has_obstacle, state.circle_hug_info.obj_index.has_value()) << msg;
			if (state.circle_hug_info.turns_to_obstacle != 0xFF || state.circle_hug_info.obj_index.has_value())
			{
				ASSERT_TRUE(villager_has_obstacle) << msg;
				const auto& ref = registry.Get<ecs::components::WallHugObjectReference>(villager_entt);
				ASSERT_EQ(ref.stepsAway, state.circle_hug_info.turns_to_obstacle) << msg;
			}

			ASSERT_NO_THROW(ecs::systems::PathfindingSystem::instance().Update()) << msg;
		}
	}

	static constexpr std::string_view scenario_path = TEST_PWD "/mobile_wall_hug/scenarios";
	std::string scene_script;
	uint32_t start_turn;
	uint32_t last_turn;
	std::vector<State> expected_states;
	std::unique_ptr<openblack::Game> game;
	entt::entity villager_entt;
};

TEST_F(MobileWallHugWalks, mobilewallhug1)
{
	MobileWallHugScenarioAssert();
}

TEST_F(MobileWallHugWalks, mobilewallhug2)
{
	MobileWallHugScenarioAssert();
}

TEST_F(MobileWallHugWalks, mobilewallhug_footpath1)
{
	MobileWallHugScenarioAssert();
}

TEST_F(MobileWallHugWalks, mobilewallhug_footpath2)
{
	MobileWallHugScenarioAssert();
}
