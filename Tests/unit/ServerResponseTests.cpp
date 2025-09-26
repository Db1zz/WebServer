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
	request.accept = "text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7";
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
	const std::string& body = response.get_body();
	EXPECT_FALSE(body.empty()) << "response body should not be empty";
	EXPECT_EQ(status.msg(), "OK") << "status message should be 'OK'";
	EXPECT_EQ(response.get_content_type(), "text/html") << "content type should be text/html";

}