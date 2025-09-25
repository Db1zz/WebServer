#include <gtest/gtest.h>

#include "Server/ServerRequestParser.hpp"
#include "Response/ServerResponse.hpp"
#include "Server/ClientSocket.hpp"
#include "Parser/Parser.hpp"


TEST(HelloTest, BasicAssertions) {
	EXPECT_STRNE("hello", "world");
	EXPECT_EQ(7 * 6, 42);
}

TEST(ServerResponseTest, SimpleGetRequest) {
	Parser parser(std::string("../Tests/unit/unit_test.conf"));
	std::vector<t_config> configs = parser.getConfigStruct();
	ASSERT_FALSE(configs.empty());
	
	t_config config = configs[0];
	ClientSocket client_socket;

	t_request& request = client_socket.get_connection_context()->request;
	request.method = "GET";
	request.protocol_version = "HTTP/1.1";
	request.uri_path = "/";
	request.uri_path_params = "";
	request.user_agent = "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/140.0.0.0 Safari/537.36";
	request.accept = "text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7";
	request.host = "localhost:90";
	request.language = "en-GB,en-US;q=0.9,en;q=0.8,ru;q=0.7,de;q=0.6";
	request.connection = "keep-alive";
	request.mime_type = "";
	request.cgi_query_string = "";
	request.content_length = 0;
	request.content_type = "";
	request.boundary = "";
	request.transfered_length = 4;
	request.filename = "/";
	request.body_chunk = "";
	request.cache = "";
	request.is_file_created = false;

	ServerResponse response(&client_socket, config);
	
	Status status;
	status = response.generate_response();
	
	EXPECT_EQ(status.code(), 200);
}