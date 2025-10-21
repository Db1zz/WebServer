#include "CGISpawner.hpp"

#include "status.hpp"
#include "CGIFileDescriptor.hpp"
#include "ClientSocket.hpp"

Status CGISpawner::spawn(ClientSocket* client_socket) {
	ConnectionContext* connection_context = client_socket->get_connection_context();
	t_request& request = connection_context->request;
	pid_t cgi_process;

	// fd[0] == read, fd[1] == write
	int server_read_pipe[2]; // this pipe is used to read stream of data from a client
	// int stdout_pipe[2];

	if (pipe(server_read_pipe) < 0) {
		perror("pipe");
		return Status("pipe() failed in create_cgi_process");
	}

	cgi_process = fork();
	if (cgi_process < 0) {
		perror("fork");
		close(server_read_pipe[0]);
		close(server_read_pipe[1]);
		return Status("fork() failed in create_cgi_process");
	}

	if (cgi_process == 0) {
		const std::string python_bin = "/usr/bin/python3";
		const std::string script_path = "./cgi-bin/aboba.py";

		std::vector<std::string> argv_strs;
		argv_strs.push_back("python3");	  // argv[0]
		argv_strs.push_back(script_path); // argv[1]
		std::vector<char*> argv;
		for (size_t i = 0; i < argv_strs.size(); ++i) {
			argv.push_back(const_cast<char*>(argv_strs[i].c_str()));
		}
		argv.push_back(NULL);

		std::vector<std::string> env_strs;
		{
			std::stringstream content_length;
			content_length << request.content_length;
			env_strs.push_back(std::string("REQUEST_METHOD=") + request.method);
			env_strs.push_back(std::string("CONTENT_LENGTH=") + content_length.str());
			env_strs.push_back(std::string("CONTENT_TYPE="));
			env_strs.push_back(std::string("SERVER_PROTOCOL=HTTP/1.1"));
			env_strs.push_back(std::string("SCRIPT_NAME="));
			env_strs.push_back(std::string("PATH_INFO="));
			env_strs.push_back(std::string("QUERY_STRING="));
			env_strs.push_back(std::string("REMOTE_ADDR="));
			env_strs.push_back(std::string("SERVER_NAME="));
			env_strs.push_back(std::string("SERVER_PORT="));
			env_strs.push_back(std::string("HTTP_USER_AGENT="));
			env_strs.push_back(std::string("HTTP_ACCEPT="));
			env_strs.push_back(std::string("SERVER_SOFTWARE=unravelThePuzzle"));
			env_strs.push_back(std::string("AUTH_TYPE=Basic"));
		}
		std::vector<char*> envp;
		for (size_t i = 0; i < env_strs.size(); ++i) {
			envp.push_back(const_cast<char*>(env_strs[i].c_str()));
		}
		envp.push_back(NULL);

		if (dup2(server_read_pipe[1], STDOUT_FILENO) < 0) {
			perror("dup2 in cgi child");
			exit(127);
		}

		close(server_read_pipe[0]);
		close(server_read_pipe[1]);

		execve(python_bin.c_str(), argv.data(), envp.data());

		perror("execve");
		exit(127);
	}

	close(server_read_pipe[1]);
	CGIFileDescriptor* descriptor = new CGIFileDescriptor(server_read_pipe[0], client_socket);
	connection_context->descriptors.insert(std::make_pair(descriptor->get_fd(), descriptor));
	return Status::OK();
}