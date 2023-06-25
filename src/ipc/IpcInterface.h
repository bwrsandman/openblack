/******************************************************************************
 * Copyright (c) 2018-2023 openblack developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/openblack/openblack
 *
 * openblack is licensed under the GNU General Public License version 3.
 *******************************************************************************/

#pragma once

#include <memory>

#include "blackhack/CameraModeNew3.h"
#include "blackhack/ControlMap.h"

namespace openblack
{
class IpcInterface
{
public:
	struct each_frame_data_camera_t
	{
		struct CameraModeNew3 mode;
		struct LHPoint pos;
		float time;
		float time_delta;
		struct Zoomer3d camera_heading_zoomer;
		struct Zoomer3d camera_origin_zoomer;
		struct Zoomer fov_zoomer;
	};

	struct each_frame_data_t
	{
		struct each_frame_data_camera_t camera;
		struct ControlMap control_map;
	};

	virtual ~IpcInterface();

	virtual bool Connect() = 0;
	virtual void Frame() = 0;
	[[nodiscard]] virtual const each_frame_data_t& GetData() const = 0;
};
} // namespace openblack
