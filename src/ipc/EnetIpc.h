/******************************************************************************
 * Copyright (c) 2018-2023 openblack developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/openblack/openblack
 *
 * openblack is licensed under the GNU General Public License version 3.
 *******************************************************************************/

#pragma once

#include "IpcInterface.h"

typedef struct _ENetHost ENetHost;
typedef struct _ENetPeer ENetPeer;

namespace openblack
{
class EnetIpc: public IpcInterface
{
public:
	virtual ~EnetIpc();
	bool Connect() override;
	void Frame() override;
	const each_frame_data_t& GetData() const override;

private:
	bool _initialized = false;
	bool _connected = false;
	ENetHost* _client = nullptr;
	ENetPeer* _peer = nullptr;
	each_frame_data_t _data;
};
} // namespace openblack
