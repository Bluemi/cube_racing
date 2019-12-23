#include "game_update_packet.hpp"

#include <iostream>

#include "../player.hpp"
#include "packet_helper.hpp"
#include "packet_ids.hpp"

namespace packet_helper {
	template<>
	void read_from_buffer<game_update_packet::player_info>(game_update_packet::player_info* obj, const char** buffer) {
		packet_helper::read_from_buffer(&obj->id, buffer);
		packet_helper::read_from_buffer(&obj->position, buffer);
		packet_helper::read_from_buffer(&obj->name, buffer);
	}

	template<>
	void write_to_buffer(const game_update_packet::player_info& pi, std::vector<char>* buffer) {
		packet_helper::write_to_buffer(pi.id, buffer);
		packet_helper::write_to_buffer(pi.position, buffer);
		packet_helper::write_to_buffer(pi.name, buffer);
	}

}

game_update_packet::player_info::player_info() {}

game_update_packet::player_info::player_info(const player& p)
	: id(p.get_id()), position(p.get_position()), name(p.get_name())
{}

game_update_packet::game_update_packet() {}

game_update_packet game_update_packet::from_players(const std::vector<player>& players, char local_player_id) {
	game_update_packet packet;
	for (const player& p : players) {
		packet._player_infos.push_back(game_update_packet::player_info(p));
	}
	packet._local_player_id = local_player_id;
	return packet;
}

game_update_packet game_update_packet::from_message(const std::vector<char>& message) {
	game_update_packet packet;

	if (message[0] != packet_ids::GAME_UPDATE_PACKET) {
		std::cerr << "wrong packet id for game_update_packet: " << (int)(message[0]) << std::endl;
	}

	const char* message_ptr = &message[1];
	const char** message_ptr_ptr = &message_ptr;
	packet_helper::read_from_buffer(&packet._player_infos, message_ptr_ptr);
	packet_helper::read_from_buffer(&packet._local_player_id, message_ptr_ptr);
	return packet;
}

void game_update_packet::write_to(std::vector<char>* buffer) const {
	buffer->push_back(packet_ids::GAME_UPDATE_PACKET);
	packet_helper::write_to_buffer(_player_infos, buffer);
	packet_helper::write_to_buffer(_local_player_id, buffer);
}

const std::vector<game_update_packet::player_info>& game_update_packet::get_player_infos() const {
	return _player_infos;
}

char game_update_packet::get_local_player_id() const {
	return _local_player_id;
}

void game_update_packet::set_local_player_id(char player_id) {
	_local_player_id = player_id;
}
