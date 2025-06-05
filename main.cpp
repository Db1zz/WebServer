#include "Sockets/ASocket.hpp"
#include "Sockets/ClientSocket.hpp"
#include "Sockets/ServerSocket.hpp"
#include "Server/Server.hpp"

int main() {
	Server server;
	server.get_socket()->set_opt(SO_REUSEADDR, true);
	server.launch();
}