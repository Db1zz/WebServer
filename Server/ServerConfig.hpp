#ifndef SERVER_SERVER_CONFIG_HPP
#define SERVER_SERVER_CONFIG_HPP

#include <map>
#include <string>
#include <vector>

typedef struct s_methods {
	bool getMethod;
	bool postMethod;
	bool deleteMethod;
} t_methods;

typedef struct s_commonConfig {
	std::string root;
	t_methods methods;
	std::vector<std::string> index;
	std::map<int, std::string> errorPage;
	bool auto_index;
	int returnCode;
	std::string returnPath;
	std::map<std::string, std::string> cgi;
	size_t max_client_body;
} t_commonConfig;

typedef struct s_location {
	std::string path;
	t_commonConfig common;
} t_location;

typedef struct s_config {
	std::vector<std::string> server_name;
	std::vector<std::string> host;
	std::vector<std::string> port;
	t_commonConfig common;
	std::vector<t_location> location;
} t_config;

#endif  // SERVER_SERVER_CONFIG_HPP