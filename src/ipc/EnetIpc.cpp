/******************************************************************************
 * Copyright (c) 2018-2023 openblack developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/openblack/openblack
 *
 * openblack is licensed under the GNU General Public License version 3.
 *******************************************************************************/

#include "EnetIpc.h"

#include <enet/enet.h>
#include <spdlog/spdlog.h>

using namespace openblack;

bool EnetIpc::Connect()
{
	const std::string host = "192.168.8.65";
	const uint16_t port = 7777;

	if (enet_initialize() != 0)
	{
		SPDLOG_LOGGER_ERROR(spdlog::get("game"), "An error occurred while initializing ENet!");
		return false;
	}

	ENetHost* client = enet_host_create(nullptr, 1, 1, 0, 0);

	if (client == nullptr)
	{
		SPDLOG_LOGGER_ERROR(spdlog::get("game"), "An error occurred while trying to create an ENet client host!");
		enet_deinitialize();
		return false;
	}

	ENetAddress address;
	enet_address_set_host(&address, host.c_str());
	address.port = port;
	ENetPeer* peer = enet_host_connect(client, &address, 1, 0);
	if (peer == nullptr)
	{
		SPDLOG_LOGGER_ERROR(spdlog::get("game"), "No available peers for initiating an ENet connection!");
		enet_host_destroy(client);
		enet_deinitialize();
		return false;
	}

	ENetEvent event;
	if (enet_host_service(client, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT)
	{
		SPDLOG_LOGGER_INFO(spdlog::get("game"), "Connection to {}:{} succeeded.", host, port);
	}
	else
	{
		enet_peer_reset(peer);
		SPDLOG_LOGGER_ERROR(spdlog::get("game"), "Connection to {}:{} failed.", host, port);
		enet_host_destroy(client);
		enet_deinitialize();
		return false;
	}

	_initialized = true;
	_connected = true;
	_client = client;
	_peer = peer;

	return _initialized;
}

EnetIpc::~EnetIpc()
{
	if (_peer != nullptr)
	{
		enet_peer_disconnect(_peer, 0);

		ENetEvent event;
		while (enet_host_service(_client, &event, 3000) > 0)
		{
			switch (event.type)
			{
			case ENET_EVENT_TYPE_RECEIVE:
				enet_packet_destroy(event.packet);
				break;
			case ENET_EVENT_TYPE_DISCONNECT:
				SPDLOG_LOGGER_INFO(spdlog::get("game"), "Disconnection succeeded.");
				break;
			default:
				break;
			}
		}
	}

	if (_client)
	{
		enet_host_destroy(_client);
	}

	if (_initialized)
	{
		enet_deinitialize();
	}
}

void EnetIpc::Frame()
{
	if (!_connected)
	{
		return;
	}
	ENetEvent event;
	while (enet_host_service(_client, &event, 1) > 0)
	{
		switch (event.type)
		{
		case ENET_EVENT_TYPE_RECEIVE:
			assert(event.packet->dataLength == sizeof(struct each_frame_data_t));
			memcpy(&_data, event.packet->data, std::min(event.packet->dataLength, sizeof(struct each_frame_data_t)));
			break;
		case ENET_EVENT_TYPE_DISCONNECT:
			SPDLOG_LOGGER_INFO(spdlog::get("game"), "Disconnected from host.");
			_connected = false;
			break;
		default:
			break;
		}
	}
}

const IpcInterface::each_frame_data_t& EnetIpc::GetData() const
{
	return _data;
}
