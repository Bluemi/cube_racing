#ifndef __SERVER_CLASS__
#define __SERVER_CLASS__

#include <netsi/server.hpp>

#include "../common/frame.hpp"
#include "../common/networking/buffer_size.hpp"

class server {
	public:
		server();

		void init();
		void run();
	private:
		struct peer_wrapper {
			peer_wrapper(const netsi::Peer& peer, const int player_id) : peer(peer), player_id(player_id), disconnected(false) {}

			netsi::Peer peer;
			char player_id;
			bool disconnected;
		};

		void check_new_peers();
		void handle_clients();
		void handle_message(const std::vector<char>& message, peer_wrapper*);
		void handle_login(const std::vector<char>& login_message, peer_wrapper*);
		void handle_logout(peer_wrapper*);
		void handle_actions(const std::vector<char>& message, peer_wrapper*);
		void send_game_update();
		void send_init(char player_id, peer_wrapper* pw) const;

		player* get_player(char player_id);

		netsi::ServerNetworkManager _server_network_manager;
		std::vector<peer_wrapper> _peers;
		frame _current_frame;
		unsigned int _next_player_id;
		unsigned int _map_seed;
};

#endif
