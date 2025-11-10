#ifndef SERVER_HTTP_RESPONSE_SENDER_HPP_
#define SERVER_HTTP_RESPONSE_SENDER_HPP_

#include "ClientSocket.hpp"
#include "ServerRequest.hpp"
#include "IResponseSender.hpp"
#include "ServerConfig.hpp"

class ServerSocketManager;

class HTTPResponseSender : public IResponseSender {
   public:
	HTTPResponseSender(ClientSocket& client_socket, t_request* request, const t_config* server_config,
					   ServerLogger* server_logger, ServerSocketManager* server_socket_manager);
	Status send();

   private:
	ClientSocket& _client_socket;
	t_request* _request;
	const t_config* _server_config;
	ServerLogger* _server_logger;
	ServerSocketManager* _server_socket_manager;
};

#endif // SERVER_HTTP_RESPONSE_SENDER_HPP_