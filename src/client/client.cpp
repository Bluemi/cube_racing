#include "client.hpp"

#include "../common/networking/login_packet.hpp"
#include "../common/networking/actions_packet.hpp"
#include "../common/networking/packet_ids.hpp"

client::client() : _local_player_id(-1) {}

void client::init(const std::string& player_name) {
	renderer::init();

	std::optional<renderer> opt_renderer = renderer::create(800, 600, "cube racing");
	if (!opt_renderer) {
		std::cerr << "Failed to initalizer renderer" << std::endl;
		return;
	}

	_renderer = std::make_unique<renderer>(*opt_renderer);

	netsi::endpoint init_endpoint = _network_manager.resolve("localhost", "1350");
	_peer = _network_manager.create_peer(init_endpoint);

	_network_manager.run();

	send_login(player_name);
}

void client::run() {
	while (!_renderer->should_close()) {
		_renderer->tick();
		if (_local_player_id != -1) {
			_renderer->render(_current_frame, _local_player_id); // TODO
		}

		while (!_peer->messages().empty()) {
			const std::vector<char> msg = _peer->messages().pop();
			handle_message(msg);
		}

		send_actions_update();
	}

	send_logout();

	_renderer->close();
	_network_manager.stop();
}

void client::send_login(const std::string& player_name) {
	login_packet packet(player_name);
	std::vector<char> buffer;
	packet.write_to(&buffer);
	_peer->async_send(buffer);
}

void client::send_logout() {
	std::vector<char> buffer;
	buffer.push_back(packet_ids::LOGOUT_PACKET);
	_peer->send(buffer);
}

void client::send_actions_update() {
	std::uint8_t current_actions(0);
	controller& ctrl = _renderer->get_controller();
	if (ctrl.is_key_pressed(controller::CAMERA_FORWARD_KEY))
		current_actions |= FORWARD_ACTION;
	if (ctrl.is_key_pressed(controller::CAMERA_BACKWARD_KEY))
		current_actions |= BACKWARD_ACTION;
	if (ctrl.is_key_pressed(controller::CAMERA_LEFT_KEY))
		current_actions |= LEFT_ACTION;
	if (ctrl.is_key_pressed(controller::CAMERA_RIGHT_KEY))
		current_actions |= RIGHT_ACTION;
	if (ctrl.is_key_pressed(controller::CAMERA_TOP_KEY))
		current_actions |= JUMP_ACTION;
	if (ctrl.is_key_pressed(controller::CAMERA_BOTTOM_KEY))
		current_actions |= BOTTOM_ACTION;

	glm::vec2 mouse_changes = ctrl.poll_mouse_changes();

	std::vector<char> buffer;
	actions_packet packet(current_actions, mouse_changes);
	packet.write_to(&buffer);
	_peer->async_send(buffer);
	_last_actions = current_actions;
}

void client::handle_message(const std::vector<char>& buffer) {
	switch (buffer[0]) {
		case packet_ids::GAME_UPDATE_PACKET:
			handle_game_update(buffer);
			break;
		default:
			std::cerr << "could not handle packet with id: " << (int)(buffer[0]) << std::endl;
	}
}

void client::apply_player_info(const game_update_packet::player_info& pi) {
	bool new_player = true;
	for (player& p : _current_frame.players) {
		if (p.get_id() == pi.id) {
			p.set_name(pi.name);
			p.set_position(pi.position);
			new_player = false;
			break;
		}
	}

	if (new_player) {
		_current_frame.players.push_back(player(pi.id, pi.name, pi.position));
	}
}

void client::handle_game_update(const std::vector<char>& buffer) {
	game_update_packet packet = game_update_packet::from_message(buffer);
	std::vector<char> current_player_ids;
	for (const game_update_packet::player_info& pi : packet.get_player_infos()) {
		apply_player_info(pi);
		current_player_ids.push_back(pi.id);
	}

	_local_player_id = packet.get_local_player_id();

	_current_frame.players.erase(
		std::remove_if(
			_current_frame.players.begin(),
			_current_frame.players.end(),
			[current_player_ids](const player& p) { return std::find(current_player_ids.begin(), current_player_ids.end(), p.get_id()) == current_player_ids.end(); }
		),
		_current_frame.players.end()
	);

	for (auto it = _current_frame.players.begin(); it != _current_frame.players.end(); ++it) {
		bool found = false;
		for (const auto& pi : packet.get_player_infos()) {
			if (pi.id == it->get_id()) {
				found = true;
				break;
			}
		}
		if (!found) {
			it = _current_frame.players.erase(it);
		}
	}
}

void print_usage() {
	std::cout << "client [playername]" << std::endl;
}


int main(int argc, const char** argv) {
	if (argc != 2) {
		print_usage();
		return 1;
	}

	std::string player_name(argv[1]);
	client c;
	c.init(player_name);
	c.run();
	return 0;
}
