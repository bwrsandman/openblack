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
