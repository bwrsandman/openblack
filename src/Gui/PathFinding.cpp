/*****************************************************************************
 * Copyright (c) 2018-2021 openblack developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/openblack/openblack
 *
 * openblack is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#include "PathFinding.h"

#include <SDL.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/vec_swizzle.hpp>
#include <imgui_internal.h>

#include <3D/LandIsland.h>
#include <Entities/Components/Mesh.h>
#include <Entities/Components/Transform.h>
#include <Entities/Components/Villager.h>
#include <Entities/Map.h>
#include <Entities/Registry.h>
#include <Game.h>

using namespace openblack::gui;

PathFinding::PathFinding()
    : DebugWindow("Path Finding", ImVec2(400, 400))
    , _handTo(HandTo::None)
    , _handPosition(glm::zero<glm::vec3>())
    , _destination(glm::zero<glm::vec3>())
    , _selectedVillager()
{
}

void PathFinding::Draw(Game& game)
{
	ImGui::TextWrapped(
	    "Select Villager by double clicking on it, then select a pathfinding action and right click the destination");

	const uint8_t numChildren = 2;
	const float rounding = 5.0f;
	const auto size = ImGui::GetWindowSize();
	const auto pos = ImGui::GetCursorPos();
	const float halfHeight = (size.y - pos.y) / numChildren - 2 * rounding;

	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, rounding);
	ImGui::BeginChild("Selected Villager", ImVec2(0, halfHeight), true);
	if (_selectedVillager.has_value())
	{
		using namespace entities::components;
		auto& registry = game.GetEntityRegistry();

		ImGui::Columns(2);

		ImGui::Text("Villager %u", static_cast<uint32_t>(*_selectedVillager));
		ImGui::NextColumn();

		if (ImGui::Button("Clear Selection"))
		{
			_selectedVillager.reset();
		}
		ImGui::NextColumn();

		ImGui::Columns(1);

		auto& transform = registry.As<Transform>(_selectedVillager.value());
		ImGui::DragFloat3("Position", glm::value_ptr(transform.position));
	}
	else
	{
		ImGui::Text("No villager selected");
	}
	ImGui::EndChild();

	ImGui::BeginChild("Actions", ImVec2(0, halfHeight), true);
	if (_selectedVillager.has_value())
	{
		if (ImGui::BeginTabBar("Actions"))
		{
			using namespace entities::components;
			auto& registry = game.GetEntityRegistry();
			if (ImGui::BeginTabItem("Teleport"))
			{
				ImGui::DragFloat3("Destination", glm::value_ptr(_destination));

				ImGui::PushStyleVar(ImGuiStyleVar_Alpha,
				                    ImGui::GetStyle().Alpha * (_selectedVillager.has_value() ? 1.0f : 0.5f));
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, !_selectedVillager.has_value());
				if (ImGui::Button("Execute"))
				{
					registry.Get<Transform>(*_selectedVillager).position = _destination;
				}
				ImGui::PopItemFlag();
				ImGui::PopStyleVar();

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Move On Footpath"))
			{
				ImGui::DragFloat3("Destination", glm::value_ptr(_destination));
				ImGui::Columns(2);
				if (_selectedFootpath.has_value())
				{
					ImGui::Text("Footpath %u", static_cast<uint32_t>(*_selectedFootpath));
				}
				else
				{
					ImGui::TextWrapped("No footpath selected, double click near one");
				}
				ImGui::NextColumn();

				if (ImGui::Button("Clear Selection"))
				{
					_selectedFootpath.reset();
				}
				ImGui::NextColumn();

				ImGui::Columns(1);

				ImGui::PushStyleVar(ImGuiStyleVar_Alpha,
				                    ImGui::GetStyle().Alpha * (_selectedFootpath.has_value() ? 1.0f : 0.5f));
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, !_selectedFootpath.has_value());
				if (ImGui::Button("Execute"))
				{
					registry.Get<Villager>(*_selectedVillager)
					    .SetupMoveOnFootpath(registry.Get<Footpath>(*_selectedFootpath), _destination);
				}
				ImGui::PopItemFlag();
				ImGui::PopStyleVar();

				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}
	}
	else
	{
		ImGui::Text("No action available");
	}
	ImGui::EndChild();

	ImGui::PopStyleVar();
}

void PathFinding::Update(Game& game, const openblack::Renderer& renderer)
{
	using namespace entities::components;
	auto& registry = game.GetEntityRegistry();
	const auto& handTransform = game.GetHandTransform();
	_handPosition = handTransform.position;
	_handPosition.y = game.GetLandIsland().GetHeightAt(glm::xz(_handPosition));

	if (_handTo == HandTo::Entity)
	{
		bool found = false;

		if (!found)
		{
			auto& map = game.GetEntityMap();
			for (const auto& entity : map.GetMobileInGridCell(_handPosition))
			{
				if (registry.Has<Villager>(entity))
				{
					const auto& mesh = registry.Get<Mesh>(entity);
					const auto& transform = registry.As<Transform>(entity);
					auto vectorTo = transform.position - _handPosition;
					if (mesh.GetBoundingBox().Contains(glm::transpose(transform.rotation) * vectorTo))
					{
						_selectedVillager = entity;
						found = true;
					}
				}
			}
		}

		if (!found)
		{
			registry.Each<Footpath>([this, &found](entt::entity entity, const Footpath& footpath) {
				if (found)
				{
					return;
				}
				for (int32_t i = 0; i < static_cast<int32_t>(footpath.nodes.size()) - 1; ++i)
				{
					const auto distance2Threshold = 4.0f;

					const auto& p1 = footpath.nodes[i].position;
					const auto& p2 = footpath.nodes[i + 1].position;
					const auto p1p2 = p2 - p1;

					const float t2 = glm::dot(_handPosition - p1, p1p2) / glm::length2(p1p2);
					const auto t = glm::clamp(t2, 0.0f, 1.0f);
					const auto proj = p1 + t * p1p2;

					if (glm::distance2(_handPosition, proj) < distance2Threshold)
					{
						_selectedFootpath = entity;
						found = true;
						return;
					}
				}
			});
		}
	}
	else if (_handTo == HandTo::Destination)
	{
		_destination = handTransform.position;
	}
	_handTo = HandTo::None;
}

void PathFinding::ProcessEventOpen(const SDL_Event& event)
{
	ImGuiIO& io = ImGui::GetIO();
	switch (event.type)
	{
	case SDL_MOUSEBUTTONDOWN:
	{
		if (!io.WantCaptureMouse)
		{
			if (event.button.button == SDL_BUTTON_LEFT && event.button.clicks == 2)
			{
				_handTo = HandTo::Entity;
			}
			else if (event.button.button == SDL_BUTTON_RIGHT)
			{
				_handTo = HandTo::Destination;
			}
		}
	}
	break;
	}
}

void PathFinding::ProcessEventAlways(const SDL_Event& event) {}
