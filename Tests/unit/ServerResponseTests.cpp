// ServerResponseTests.cpp
#include <gtest/gtest.h>

#include "Parser/Parser.hpp"
#include "Server/RequestParser/ServerRequestParser.hpp"
#include "Response/ServerResponse.hpp"
#include "Server/ServerRequest.hpp"
#include "Server/ClientSocket.hpp"
#include "Server/IOClientContext.hpp"
#include "Server/ServerSocket.hpp"
#include "Server/ServerSocketManager.hpp"
#include "Logger/ServerLogger.hpp"

// Если ваша реализация требует реальный ServerEvent — создайте объект и передайте адрес вместо nullptr.
static ServerEvent* g_server_event_ptr = nullptr;

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
        request.user_agent = "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/140.0.0.0 Safari/537.36";
        request.accept = "text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7";
        request.host = "localhost:90";
        request.language = "en-GB,en-US;q=0.9,en;q=0.8,ru;q=0.7,de;q=0.6";
        request.connection = "keep-alive";
        request.mime_type = "";
        request.content_length = 0;
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

#define SETUP_COMMON \
    ServerLogger server_logger("./test_logs"); \
    ServerSocket server_socket("127.0.0.1", 90); \
    ServerSocketManager server_socket_manager("127.0.0.1", 90, g_server_event_ptr, config, &server_logger); \
    ClientSocket client_socket(&config); \
    IOClientContext io_context(client_socket, server_socket, server_socket_manager, &config, &server_logger);

TEST_F(ServerResponseTest, SimpleGetRequest) {
    SETUP_COMMON
    io_context.request = createBaseRequest();

    ServerResponse response(&io_context, config);
    Status status = response.generate_response();

    EXPECT_EQ(status.code(), 200);
    const std::string& body = response.get_body();
    EXPECT_FALSE(body.empty()) << "response body should not be empty";
    EXPECT_EQ(status.msg(), "OK") << "status message should be 'OK'";
    EXPECT_EQ(response.get_content_type(), "text/html") << "content type should be text/html";
}

TEST_F(ServerResponseTest, NonExistentPath) {
    SETUP_COMMON
    io_context.request = createBaseRequest();
    io_context.request.uri_path = "/nonexistent";
    io_context.request.filename = "/nonexistent";

    ServerResponse response(&io_context, config);
    Status status = response.generate_response();
    EXPECT_EQ(status.code(), 404);
}

TEST_F(ServerResponseTest, NonExistentPathNoFallBack) {
    SETUP_COMMON
    io_context.request = createBaseRequest();
    io_context.request.uri_path = "/nonexistent";
    io_context.request.filename = "/nonexistent";
    config.common.errorPage[404] = "nonexistent_error_page.html";

    ServerResponse response(&io_context, config);
    Status status = response.generate_response();
    EXPECT_EQ(status.code(), 404);
}

TEST_F(ServerResponseTest, UploadHtmlRequest) {
    SETUP_COMMON
    io_context.request = createBaseRequest();
    io_context.request.uri_path = "/upload.html";
    io_context.request.mime_type = ".html";

    ServerResponse response(&io_context, config);
    Status status = response.generate_response();

    EXPECT_EQ(status.code(), 200);
    EXPECT_EQ(response.get_content_type(), "text/html");
}

TEST_F(ServerResponseTest, StylesCssRequest) {
    SETUP_COMMON
    io_context.request = createBaseRequest();
    io_context.request.uri_path = "/styles.css";
    io_context.request.mime_type = ".css";

    ServerResponse response(&io_context, config);
    Status status = response.generate_response();

    EXPECT_EQ(status.code(), 200);
    EXPECT_EQ(response.get_content_type(), "text/css");
}

TEST_F(ServerResponseTest, JavaScriptRequest) {
    SETUP_COMMON
    io_context.request = createBaseRequest();
    io_context.request.uri_path = "/upload.js";
    io_context.request.accept = "*/*";
    io_context.request.mime_type = ".js";

    ServerResponse response(&io_context, config);
    Status status = response.generate_response();

    EXPECT_EQ(status.code(), 200);
    EXPECT_EQ(response.get_content_type(), "application/javascript");
}

TEST_F(ServerResponseTest, UploadsDirectoryRequest) {
    SETUP_COMMON
    io_context.request = createBaseRequest();
    io_context.request.uri_path = "/Uploads/";
    io_context.request.accept = "*/*";
    io_context.request.mime_type = "";
    io_context.request.filename = "/";

    ServerResponse response(&io_context, config);
    Status status = response.generate_response();

    EXPECT_EQ(status.code(), 200);
}

TEST_F(ServerResponseTest, PostFileRequest) {
    SETUP_COMMON
    io_context.request = createBaseRequest();
    io_context.request.method = "POST";
    io_context.request.uri_path = "/Uploads/";
    io_context.request.accept = "*/*";
    io_context.request.mime_type = ".txt";
    io_context.request.content_length = 184;
    io_context.request.content_type.type = "multipart";
    io_context.request.content_type.subtype = "form-data";
    io_context.request.content_type.parameters.insert({"boundary","----WebKitFormBoundaryTj2MepeomC2UszbC"});
    io_context.request.transfered_length = 184;

    t_request_content content;
    content.filename = "fileuploadtest.txt";
    content.name = "file";
    content.content_type = "text/plain";
    content.data = "This is test file content for upload testing.";
    content.is_file_created = false;
    content.is_finished = true;
    io_context.request.content_data.push_back(content);

    ServerResponse response(&io_context, config);
    Status status = response.generate_response();

    EXPECT_EQ(status.code(), 200);
}

TEST_F(ServerResponseTest, PostDuplicateFileRequest) {
    SETUP_COMMON
    io_context.request = createBaseRequest();
    io_context.request.method = "POST";
    io_context.request.uri_path = "/Uploads/";
    io_context.request.accept = "*/*";
    io_context.request.mime_type = ".txt";
    io_context.request.content_length = 184;
    io_context.request.content_type.type = "multipart";
    io_context.request.content_type.subtype = "form-data";
    io_context.request.content_type.parameters.insert({"boundary","----WebKitFormBoundaryTj2MepeomC2UszbC"});
    io_context.request.transfered_length = 184;

    t_request_content content;
    content.filename = "fileuploadtest.txt";
    content.name = "file";
    content.content_type = "text/plain";
    content.data = "This is duplicate file content.";
    content.is_file_created = false;
    content.is_finished = true;
    io_context.request.content_data.push_back(content);

    ServerResponse response(&io_context, config);
    Status status = response.generate_response();

    EXPECT_EQ(status.code(), 409);
}

TEST_F(ServerResponseTest, PostChunkedUploadRequest) {
    SETUP_COMMON
    io_context.request = createBaseRequest();
    io_context.request.method = "POST";
    io_context.request.uri_path = "/Uploads/";
    io_context.request.accept = "*/*";
    io_context.request.mime_type = ".txt";
    io_context.request.content_length = 500;
    io_context.request.content_type.type = "multipart";
    io_context.request.content_type.subtype = "form-data";
    io_context.request.content_type.parameters.insert({"boundary","----WebKitFormBoundaryTj2MepeomC2UszbC"});

    t_request_content content;
    content.filename = "chunkedfile.txt";
    content.name = "file";
    content.content_type = "text/plain";
    content.data = "This is first chunk of data...";
    content.is_file_created = false;
    content.is_finished = false;
    io_context.request.content_data.push_back(content);

    ServerResponse response(&io_context, config);
    Status status = response.generate_response();

    EXPECT_EQ(status.code(), 100);
}

TEST_F(ServerResponseTest, DeleteChunkedFileRequest) {
    SETUP_COMMON
    io_context.request = createBaseRequest();
    io_context.request.method = "DELETE";
    io_context.request.uri_path = "/Uploads/chunkedfile.txt";
    io_context.request.accept = "*/*";
    io_context.request.mime_type = ".txt";
    io_context.request.filename = "chunkedfile.txt";

    ServerResponse response(&io_context, config);
    Status status = response.generate_response();

    EXPECT_EQ(status.code(), 200);
}

TEST_F(ServerResponseTest, UploadTwoFilesOneLineEach) {
    SETUP_COMMON
    io_context.request = createBaseRequest();
    io_context.request.method = "POST";
    io_context.request.uri_path = "/Uploads/";
    io_context.request.accept = "*/*";
    io_context.request.mime_type = ".txt";
    io_context.request.content_length = 300;
    io_context.request.content_type.type = "multipart";
    io_context.request.content_type.subtype = "form-data";
    io_context.request.content_type.parameters.insert({"boundary","----WebKitFormBoundaryTj2MepeomC2UszbC"});
    io_context.request.transfered_length = 300;

    t_request_content content1;
    content1.filename = "firstline.txt";
    content1.name = "file1";
    content1.content_type = "text/plain";
    content1.data = "First file with single line content.";
    content1.is_file_created = false;
    content1.is_finished = true;

    t_request_content content2;
    content2.filename = "secondline.txt";
    content2.name = "file2";
    content2.content_type = "text/plain";
    content2.data = "Second file with single line content.";
    content2.is_file_created = false;
    content2.is_finished = true;

    io_context.request.content_data.push_back(content1);
    io_context.request.content_data.push_back(content2);

    ServerResponse response(&io_context, config);
    Status status = response.generate_response();

    EXPECT_EQ(status.code(), 200);
}

TEST_F(ServerResponseTest, UploadMixedSizeFiles) {
    SETUP_COMMON
    io_context.request = createBaseRequest();
    io_context.request.method = "POST";
    io_context.request.uri_path = "/Uploads/";
    io_context.request.accept = "*/*";
    io_context.request.mime_type = ".txt";
    io_context.request.content_length = 1048650;
    io_context.request.content_type.type = "multipart";
    io_context.request.content_type.subtype = "form-data";
    io_context.request.content_type.parameters.insert({"boundary","----WebKitFormBoundaryTj2MepeomC2UszbC"});
    io_context.request.transfered_length = 1048650;

    t_request_content content1;
    content1.filename = "smallfile.txt";
    content1.name = "file1";
    content1.content_type = "text/plain";
    content1.data = "Small file with just one line.";
    content1.is_file_created = false;
    content1.is_finished = true;

    t_request_content content2;
    content2.filename = "bigfile.txt";
    content2.name = "file2";
    content2.content_type = "text/plain";
    content2.data = std::string(1048600, 'L');
    content2.is_file_created = false;
    content2.is_finished = true;

    io_context.request.content_data.push_back(content1);
    io_context.request.content_data.push_back(content2);

    ServerResponse response(&io_context, config);
    Status status = response.generate_response();

    EXPECT_EQ(status.code(), 200);
}

TEST_F(ServerResponseTest, UploadFilesWithDuplicate) {
    SETUP_COMMON
    io_context.request = createBaseRequest();
    io_context.request.method = "POST";
    io_context.request.uri_path = "/Uploads/";
    io_context.request.accept = "*/*";
    io_context.request.mime_type = ".txt";
    io_context.request.content_length = 400;
    io_context.request.content_type.type = "multipart";
    io_context.request.content_type.subtype = "form-data";
    io_context.request.content_type.parameters.insert({"boundary","----WebKitFormBoundaryTj2MepeomC2UszbC"});
    io_context.request.transfered_length = 400;

    t_request_content content1;
    content1.filename = "uniquefile.txt";
    content1.name = "file1";
    content1.content_type = "text/plain";
    content1.data = "Unique file content for duplicate test.";
    content1.is_file_created = false;
    content1.is_finished = true;

    t_request_content content2;
    content2.filename = "fileuploadtest.txt";
    content2.name = "file2";
    content2.content_type = "text/plain";
    content2.data = "Attempting to upload duplicate file.";
    content2.is_file_created = false;
    content2.is_finished = true;

    io_context.request.content_data.push_back(content1);
    io_context.request.content_data.push_back(content2);

    ServerResponse response(&io_context, config);
    Status status = response.generate_response();

    EXPECT_EQ(status.code(), 409);
}

TEST_F(ServerResponseTest, DeleteFileRequest) {
    SETUP_COMMON
    io_context.request = createBaseRequest();
    io_context.request.method = "DELETE";
    io_context.request.uri_path = "/Uploads/fileuploadtest.txt";
    io_context.request.accept = "*/*";
    io_context.request.mime_type = ".txt";
    io_context.request.filename = "fileuploadtest.txt";

    ServerResponse response(&io_context, config);
    Status status = response.generate_response();

    EXPECT_EQ(status.code(), 200);
}

TEST_F(ServerResponseTest, DeleteFirstLineFile) {
    SETUP_COMMON
    io_context.request = createBaseRequest();
    io_context.request.method = "DELETE";
    io_context.request.uri_path = "/Uploads/firstline.txt";
    io_context.request.accept = "*/*";
    io_context.request.mime_type = ".txt";
    io_context.request.filename = "firstline.txt";

    ServerResponse response(&io_context, config);
    Status status = response.generate_response();

    EXPECT_EQ(status.code(), 200);
}

TEST_F(ServerResponseTest, DeleteSecondLineFile) {
    SETUP_COMMON
    io_context.request = createBaseRequest();
    io_context.request.method = "DELETE";
    io_context.request.uri_path = "/Uploads/secondline.txt";
    io_context.request.accept = "*/*";
    io_context.request.mime_type = ".txt";
    io_context.request.filename = "secondline.txt";

    ServerResponse response(&io_context, config);
    Status status = response.generate_response();

    EXPECT_EQ(status.code(), 200);
}

TEST_F(ServerResponseTest, DeleteSmallFile) {
    SETUP_COMMON
    io_context.request = createBaseRequest();
    io_context.request.method = "DELETE";
    io_context.request.uri_path = "/Uploads/smallfile.txt";
    io_context.request.accept = "*/*";
    io_context.request.mime_type = ".txt";
    io_context.request.filename = "smallfile.txt";

    ServerResponse response(&io_context, config);
    Status status = response.generate_response();

    EXPECT_EQ(status.code(), 200);
}

TEST_F(ServerResponseTest, DeleteBigFile) {
    SETUP_COMMON
    io_context.request = createBaseRequest();
    io_context.request.method = "DELETE";
    io_context.request.uri_path = "/Uploads/bigfile.txt";
    io_context.request.accept = "*/*";
    io_context.request.mime_type = ".txt";
    io_context.request.filename = "bigfile.txt";

    ServerResponse response(&io_context, config);
    Status status = response.generate_response();

    EXPECT_EQ(status.code(), 200);
}

TEST_F(ServerResponseTest, DeleteUniqueFile) {
    SETUP_COMMON
    io_context.request = createBaseRequest();
    io_context.request.method = "DELETE";
    io_context.request.uri_path = "/Uploads/uniquefile.txt";
    io_context.request.accept = "*/*";
    io_context.request.mime_type = ".txt";
    io_context.request.filename = "uniquefile.txt";

    ServerResponse response(&io_context, config);
    Status status = response.generate_response();

    EXPECT_EQ(status.code(), 200);
}
