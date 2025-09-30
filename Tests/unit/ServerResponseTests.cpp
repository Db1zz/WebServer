#include <gtest/gtest.h>

#include "Server/RequestParser/ServerRequestParser.hpp"
#include "Response/ServerResponse.hpp"
#include "Server/ServerRequest.hpp"
#include "Server/ClientSocket.hpp"
#include "Parser/Parser.hpp"

class ServerResponseTest : public ::testing::Test {
protected:
	static std::vector<t_config> configs;
	static t_config config;

	static void SetUpTestSuite() {
		Parser parser(std::string("../Tests/unit/unit_test.conf"));
		configs = parser.getConfigStruct();
		ASSERT_FALSE(configs.empty());
		config = configs[0];
	}

	static t_request createBaseRequest() {
		t_request request;
		request.method = "GET";
		request.protocol_version = "HTTP/1.1";
		request.uri_path = "/";
		// request.uri_path_params = "";
		request.user_agent = "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/140.0.0.0 Safari/537.36";
		request.accept = "text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7";
		request.host = "localhost:90";
		request.language = "en-GB,en-US;q=0.9,en;q=0.8,ru;q=0.7,de;q=0.6";
		request.connection = "keep-alive";
		request.mime_type = "";
		// request.cgi_query_string = "";
		request.content_length = 0;
		// request.content_type = "";
		request.transfered_length = 4;
		request.filename = "/";
		request.body_chunk = "";
		request.cache = "";
		request.is_file_created = false;
		return request;
	}
};

std::vector<t_config> ServerResponseTest::configs;
t_config ServerResponseTest::config;

TEST(HelloTest, BasicAssertions) {
	EXPECT_STRNE("hello", "world");
	EXPECT_EQ(7 * 6, 42);
}

TEST_F(ServerResponseTest, SimpleGetRequest) {
	ClientSocket client_socket;
	t_request& request = client_socket.get_connection_context()->request;
	request = createBaseRequest();

	ServerResponse response(&client_socket, config);

	Status status;
	status = response.generate_response();
	
	EXPECT_EQ(status.code(), 200);
	const std::string& body = response.get_body();
	EXPECT_FALSE(body.empty()) << "response body should not be empty";
	EXPECT_EQ(status.msg(), "OK") << "status message should be 'OK'";
	EXPECT_EQ(response.get_content_type(), "text/html") << "content type should be text/html";
}


TEST_F(ServerResponseTest, NonExistentPath) {
	ClientSocket client_socket;

	t_request& request = client_socket.get_connection_context()->request;
	request = createBaseRequest();
	request.uri_path = "/nonexistent";
	request.filename = "/nonexistent";

	ServerResponse response(&client_socket, config);
	
	Status status;
	status = response.generate_response();
	
	EXPECT_EQ(status.code(), 404);
}

TEST_F(ServerResponseTest, NonExistentPathNoFallBack) {
	ClientSocket client_socket;

	t_request& request = client_socket.get_connection_context()->request;
	request = createBaseRequest();
	request.uri_path = "/nonexistent";
	request.filename = "/nonexistent";

	config.common.errorPage[404] = "nonexistent_error_page.html";

	ServerResponse response(&client_socket, config);
	
	Status status;
	status = response.generate_response();
	
	EXPECT_EQ(status.code(), 404);
}


TEST_F(ServerResponseTest, UploadHtmlRequest) {
	ClientSocket client_socket;

	t_request& request = client_socket.get_connection_context()->request;
	request = createBaseRequest();
	request.uri_path = "/upload.html";
	request.mime_type = ".html";

	ServerResponse response(&client_socket, config);
	
	Status status;
	status = response.generate_response();
	
	EXPECT_EQ(status.code(), 200);
	EXPECT_EQ(response.get_content_type(), "text/html");
}

TEST_F(ServerResponseTest, StylesCssRequest) {
	ClientSocket client_socket;

	t_request& request = client_socket.get_connection_context()->request;
	request = createBaseRequest();
	request.uri_path = "/styles.css";
	request.mime_type = ".css";

	ServerResponse response(&client_socket, config);
	
	Status status;
	status = response.generate_response();
	EXPECT_EQ(status.code(), 200);
	EXPECT_EQ(response.get_content_type(), "text/css");
}

TEST_F(ServerResponseTest, JavaScriptRequest) {
	ClientSocket client_socket;

	t_request& request = client_socket.get_connection_context()->request;
	request = createBaseRequest();
	request.uri_path = "/upload.js";
	request.accept = "*/*";
	request.mime_type = ".js";

	ServerResponse response(&client_socket, config);
	
	Status status;
	status = response.generate_response();
	
	EXPECT_EQ(status.code(), 200);
	EXPECT_EQ(response.get_content_type(), "application/javascript");
}

TEST_F(ServerResponseTest, UploadsDirectoryRequest) {
	ClientSocket client_socket;

	t_request& request = client_socket.get_connection_context()->request;
	request = createBaseRequest();
	request.uri_path = "/Uploads/";
	request.accept = "*/*";
	request.mime_type = "";
	request.filename = "/";

	ServerResponse response(&client_socket, config);
	
	Status status;
	status = response.generate_response();
	
	EXPECT_EQ(status.code(), 200);
}

TEST_F(ServerResponseTest, PostFileRequest) {
	ClientSocket client_socket;

	t_request& request = client_socket.get_connection_context()->request;
	request = createBaseRequest();
	request.method = "POST";
	request.uri_path = "/Uploads/";
	request.accept = "*/*";
	request.mime_type = ".txt";
	request.content_length = 184;
	request.content_type.type = "multipart";
	request.content_type.subtype = "form-data";
	request.content_type.parameters.insert({"boundary","----WebKitFormBoundaryTj2MepeomC2UszbC"});
	request.transfered_length = 184;
	request.filename = "fileuploadtest.txt";
	request.is_file_created = false;

	ServerResponse response(&client_socket, config);
	
	Status status;
	status = response.generate_response();
	
	EXPECT_EQ(status.code(), 200);
}

TEST_F(ServerResponseTest, PostDuplicateFileRequest) {
	ClientSocket client_socket;

	t_request& request = client_socket.get_connection_context()->request;
	request = createBaseRequest();
	request.method = "POST";
	request.uri_path = "/Uploads/";
	request.accept = "*/*";
	request.mime_type = ".txt";
	request.content_length = 184;
	request.content_type.type = "multipart";
	request.content_type.subtype = "form-data";
	request.content_type.parameters.insert({"boundary","----WebKitFormBoundaryTj2MepeomC2UszbC"});
	request.transfered_length = 184;
	request.filename = "fileuploadtest.txt";
	request.is_file_created = false;

	ServerResponse response(&client_socket, config);
	
	Status status;
	status = response.generate_response();
	
	EXPECT_EQ(status.code(), 409);
}

TEST_F(ServerResponseTest, DeleteFileRequest) {
	ClientSocket client_socket;

	t_request& request = client_socket.get_connection_context()->request;
	request = createBaseRequest();
	request.method = "DELETE";
	request.uri_path = "/Uploads/fileuploadtest.txt";
	request.accept = "*/*";
	request.mime_type = ".txt";
	request.filename = "fileuploadtest.txt";

	ServerResponse response(&client_socket, config);
	
	Status status;
	status = response.generate_response();
	
	EXPECT_EQ(status.code(), 200);
}
