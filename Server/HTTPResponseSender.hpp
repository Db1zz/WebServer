#ifndef SERVER_HTTP_RESPONSE_SENDER_HPP_
#define SERVER_HTTP_RESPONSE_SENDER_HPP_

#include "ClientSocket.hpp"
#include "ServerRequest.hpp"
#include "IResponseSender.hpp"
#include "ServerConfig.hpp"

class ServerSocket;

class HTTPResponseSender : public IResponseSender {
   public:
	HTTPResponseSender(ClientSocket& client_socket, t_request* request, const t_config* server_config,
					   ServerSocket& server_socket, ServerLogger* server_logger);
	Status send(const Status& status);

   private:
	ClientSocket& _client_socket;
	t_request* _request;
	const t_config* _server_config;
	ServerLogger* _server_logger;
	ServerSocket* _server_socket;
};

#endif // SERVER_HTTP_RESPONSE_SENDER_HPP_