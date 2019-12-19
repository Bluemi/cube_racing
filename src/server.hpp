#ifndef __SERVER_CLASS__
#define __SERVER_CLASS__

#include <netsi/util/cycle.hpp>
#include <netsi/server/server_network_manager.hpp>

#include "frame.hpp"
#include "render/renderer.hpp"

class server {
	public:
		server();

		void init();
		void run();
	private:
		void check_new_peers();
		void handle_to_clients();

		netsi::server_network_manager _server_network_manager;
		std::vector<std::shared_ptr<netsi::peer>> _peers;
		std::unique_ptr<renderer> _renderer;
		frame _current_frame;
};

#endif
