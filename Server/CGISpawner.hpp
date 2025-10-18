#ifndef SERVER_SERVER_CGI_SPAWNER_HPP_
#define SERVER_SERVER_CGI_SPAWNER_HPP_

class ClientSocket;
class Status;

class CGISpawner {
public:
	CGISpawner() {}
	Status spawn(ClientSocket* client_socket);
};

#endif  // SERVER_SERVER_CGI_SPAWNER_HPP_