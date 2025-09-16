#include <gtest/gtest.h>

#include <string>

#include "Server/ServerRequest.hpp"
#include "Server/ServerRequestParser.hpp"
#include "Server/ServerRequestParserHelpers.hpp"
#include "Utilities/status.hpp"

TEST(ParseRequestBodyHeaderTests, HeaderDoesNotExist) {
	Status status;
	std::string cache =
		"\r\n\r\n"
		"content\r\n"
		"--aboba123--\r\n";
	std::string boundary_start = "--aboba123\r\n";
	size_t pos = 0;
	size_t transfered_length = 0;
	std::string result;

	status = internal_server_request_parser::parse_request_body_header(
		cache, pos, transfered_length, boundary_start, result);
	EXPECT_EQ(status, false);
}

TEST(ParseRequestBodyHeaderTests, HeaderExists) {
	Status status;
	std::string cache =
		"--aboba123\r\n"
		"\r\n"
		"content\r\n"
		"--aboba123--\r\n";

	std::string boundary_start = "--aboba123\r\n";
	size_t pos = 0;
	size_t transfered_length = 0;
	std::string result;

	status = internal_server_request_parser::parse_request_body_header(
		cache, pos, transfered_length, boundary_start, result);
	std::cout << "ABOBA: " << status.msg() << std::endl;
	EXPECT_EQ(status, true);
}

TEST(ParseRequestBodyHeaderTests, BoundaryWithTypoo) {
	Status status;
	std::string cache =
		"--aboba124\r\n"
		"\r\n"
		"content\r\n"
		"--aboba123--\r\n";
	std::string boundary_start = "--aboba123\r\n";
	size_t pos = 0;
	size_t transfered_length = 0;
	std::string result;

	status = internal_server_request_parser::parse_request_body_header(
		cache, pos, transfered_length, boundary_start, result);
	EXPECT_EQ(status, false);
}

TEST(ParseRequestBodyHeaderTests, BoundaryWithRandomPrefix) {
	Status status;
	std::string cache =
		"\r\nhfakjdhfakdfhkajs--aboba123\r\n"
		"\r\n"
		"content\r\n"
		"--aboba123--\r\n";
	std::string boundary_start = "\r\n--aboba123\r\n";
	size_t pos = 0;
	size_t transfered_length = 0;
	std::string result;

	status = internal_server_request_parser::parse_request_body_header(
		cache, pos, transfered_length, boundary_start, result);
	EXPECT_EQ(status, false);
}

TEST(ParseRequestBodyHeaderTests, OnlyEndBoundary) {
	Status status;
	std::string cache =
		"\r\n\r\n"
		"content\r\n"
		"--aboba123--\r\n";
	std::string boundary_start = "\r\n--aboba123\r\n";
	size_t pos = 0;
	size_t transfered_length = 0;
	std::string result;

	status = internal_server_request_parser::parse_request_body_header(
		cache, pos, transfered_length, boundary_start, result);
	EXPECT_EQ(status, false);
}

TEST(ParseRequestBodyChunk, EndBoundaryExist) {
	Status status;
	std::string cache =
		"\r\n--aboba123\r\n\r\n"
		"content\r\n"
		"--aboba123--\r\n";
	std::string boundary_start = "\r\n--aboba123\r\n";
	std::string boundary_end = "\r\n--aboba123--\r\n";
	size_t pos = 0;
	size_t transfered_length = 0;
	std::string result;

	status = internal_server_request_parser::parse_request_body_header(
		cache, pos, transfered_length, boundary_start, result);
	std::cout << status.msg() << std::endl;
	EXPECT_EQ(status, true);
	status = internal_server_request_parser::parse_request_body_chunk(cache, pos, transfered_length,
																	  boundary_end, result);
	std::cout << status.msg() << std::endl;
	EXPECT_EQ(status, true);
}

TEST(ParseRequestBodyChunk, WrongEndBoundary) {
	Status status;
	std::string cache =
		"\r\n--aboba123\r\n\r\n"
		"content\r\n"
		"--aboba123\r\n";
	std::string boundary_start = "\r\n--aboba123\r\n";
	std::string boundary_end = "\r\n--aboba123--\r\n";
	size_t pos = 0;
	size_t transfered_length = 0;
	std::string result;

	status = internal_server_request_parser::parse_request_body_header(
		cache, pos, transfered_length, boundary_start, result);
	EXPECT_EQ(status, true);
	status = internal_server_request_parser::parse_request_body_chunk(cache, pos, transfered_length,
																	  boundary_end, result);
	EXPECT_EQ(status, false);
}

TEST(ParseRequestBodyChunk, IncompleteEndBoundary) {
	Status status;
	std::string cache =
		"\r\n--aboba123\r\n\r\n"
		"content\r\n"
		"--aboba123";
	std::string boundary_start = "\r\n--aboba123\r\n";
	std::string boundary_end = "\r\n--aboba123--\r\n";
	size_t pos = 0;
	size_t transfered_length = 0;
	std::string result;

	status = internal_server_request_parser::parse_request_body_header(
		cache, pos, transfered_length, boundary_start, result);
	EXPECT_EQ(status, true);
	status = internal_server_request_parser::parse_request_body_chunk(cache, pos, transfered_length,
																	  boundary_end, result);
	EXPECT_EQ(status, false);
}

TEST(ParseRequestBodyChunk, ResultAndCacheCorrectness) {
	Status status;
	std::string cache =
		"\r\n--aboba123\r\n\r\n"
		"content\r\n"
		"--aboba123--\r\n"
		"data";
	std::string boundary_start = "\r\n--aboba123\r\n";
	std::string boundary_end = "\r\n--aboba123--\r\n";
	size_t pos = 0;
	size_t transfered_length = 0;
	std::string result;

	status = internal_server_request_parser::parse_request_body_header(
		cache, pos, transfered_length, boundary_start, result);
	EXPECT_EQ(status, true);
	status = internal_server_request_parser::parse_request_body_chunk(cache, pos, transfered_length,
																	  boundary_end, result);
	EXPECT_EQ(status, true);
	EXPECT_EQ(cache, "data");
	EXPECT_EQ(result, "content");
}

TEST(ParseRequestBodyChunk, ResultAndCacheCorrectnessOfTheIncompleteBody) {
	Status status;
	std::string cache =
		"\r\n--aboba123\r\n\r\n"
		"content\r\n"
		"--aboba123";
	std::string boundary_start = "\r\n--aboba123\r\n";
	std::string boundary_end = "\r\n--aboba123--\r\n";
	size_t pos = 0;
	size_t transfered_length = 0;
	std::string result;

	status = internal_server_request_parser::parse_request_body_header(
		cache, pos, transfered_length, boundary_start, result);
	EXPECT_EQ(status, true);
	status = internal_server_request_parser::parse_request_body_chunk(cache, pos, transfered_length,
																	  boundary_end, result);
	EXPECT_EQ(status, false);
	EXPECT_EQ(cache, "tent\r\n--aboba123");
	EXPECT_EQ(result, "con");
}

TEST(GetTokenWithDelim, FindExsistingDelim) {
	size_t pos = 0;
	std::string data = "GET ./aboba.html HTTP/1.1\r\n";
	std::string result;

	internal_server_request_parser::get_token_with_delim(data, pos, result, "\r\n");
	EXPECT_EQ(result.empty(), false);
	EXPECT_EQ(result, "GET ./aboba.html HTTP/1.1");
}

TEST(GetTokenWithDelim, FindNonexsitingDelim) {
	size_t pos = 0;
	std::string data = "GET ./aboba.html HTTP/1.1";
	std::string result;

	internal_server_request_parser::get_token_with_delim(data, pos, result, "\r\n");
	EXPECT_EQ(result.empty(), false);
	EXPECT_EQ(result, "GET ./aboba.html HTTP/1.1");
}

TEST(GetTokenWithDelim, SplitStringWithDelims) {
	size_t pos = 0;
	std::string data = "GET ./aboba.html HTTP/1.1\r\nsdhfkjhasdf";
	std::string method;
	std::string filename;
	std::string protocol;

	pos = internal_server_request_parser::get_token_with_delim(data, pos, method, " ");
	pos = internal_server_request_parser::get_token_with_delim(data, pos, filename, " ");
	pos = internal_server_request_parser::get_token_with_delim(data, pos, protocol, "\r\n");
	EXPECT_EQ(method, "GET");
	EXPECT_EQ(filename, "./aboba.html");
	EXPECT_EQ(protocol, "HTTP/1.1");
}

TEST(ParseFileNameWithMime, ParseExsitingFilename) {
	Status status;
	std::string full_path = "/aboba/path/my.aboba";
	std::string filename;
	std::string mime;

	status = internal_server_request_parser::parse_file_name_with_mime(full_path, filename, mime);
	EXPECT_EQ(status, true);
	EXPECT_EQ(filename, "/my.aboba");
	EXPECT_EQ(mime, ".aboba");
}

TEST(ServerRequestParserTest, ParseCompleteGETRequest) {
	Status status;
	t_request request;
	ServerRequestParser parser(&request);

	std::string request_string =
		"GET ./aboba.html HTTP/1.1\r\n"
		"Host: aboba.app\r\n"
		"User-Agent: Mozilla/5.0 (AbobaChips; Quantum Computer X 10.15; rv:142.0) Gecko/1337 "
		"Firefox/142.0\r\n"
		"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
		"Accept-Language: en-GB,en;q=0.5\r\n"
		"Accept-Encoding: gzip, deflate, br, zstd\r\n"
		"Connection: keep-alive\r\n"
		"Priority: u=0, i\r\n"
		"\r\n";

	status = parser.parse_chunk(request_string);
	EXPECT_EQ(status, true);
	EXPECT_EQ(request.method, "GET");
	EXPECT_EQ(request.protocol_version, "HTTP/1.1");
	EXPECT_EQ(request.filename, "/aboba.html");
	EXPECT_EQ(request.host, "aboba.app");
	EXPECT_EQ(request.accept, "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
	EXPECT_EQ(request.connection, "keep-alive");
}

TEST(ServerRequestParserTest, ParseIncompleteGETRequest) {
	Status status;
	t_request request;
	ServerRequestParser parser(&request);

	std::string request_string =
		"GET ./aboba.html HTTP/1.1\r\n"
		"Host: aboba.app\r\n"
		"User-Agent: Mozilla/5.0 (AbobaChips; Quantum Computer X 10.15; rv:142.0) Gecko/1337 "
		"Firefox/142.0\r\n"
		"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
		"Accept-Language: en-GB,en;q=0.5\r\n"
		"Accept-Encoding: gzip, deflate, br, zstd\r\n"
		"Connection: keep-alive\r\n"
		"Priority: u=0, i\r\n";

	status = parser.parse_chunk(request_string);
	EXPECT_EQ(status, false);
}

TEST(ServerRequestParserTest, ParseCompletePostRequest) {
	Status status;
	t_request request;
	ServerRequestParser parser(&request);

	std::string request_string =
		"POST /upload HTTP/1.1\r\n"
		"Host: example.com\r\n"
		"Content-Type: multipart/form-data; boundary=aboba123\r\n"
		"Content-Length: 146\r\n"
		"\r\n"
		"--aboba123\r\n"
		"Content-Disposition: form-data; name=\"file\"; filename=\"example.txt\"\r\n"
		"Content-Type: text/plain\r\n"
		"\r\n"
		"(file contents here)\r\n"
		"--aboba123--\r\n";

	status = parser.parse_chunk(request_string);
	EXPECT_EQ(request.body_chunk, "(file contents here)");
	EXPECT_EQ(request.content_length, 146);
	EXPECT_EQ(request.filename, "example.txt");
	EXPECT_EQ(request.mime_type, ".txt");
}

TEST(ServerRequestParserTest, ParsePartitionedPostRequest) {
	Status status;
	t_request request;
	ServerRequestParser parser(&request);

	std::string request_string =
		"POST /upload HTTP/1.1\r\n"
		"Host: example.com\r\n"
		"Content-Type: multipart/form-data; boundary=aboba123\r\n"
		"Content-Length: 148\r\n"
		"\r\n"
		"--aboba123\r\n"
		"Content-Disposition: form-data; name=\"file\"; filename=\"example.txt\"\r\n"
		"Content-Type: text/plain\r\n"
		"\r\n"
		"(some huge filecontent pizdec nahuy 12";

	status = parser.parse_chunk(request_string);
	std::cout << "Status: " << status.msg() << std::endl;
	EXPECT_EQ(request.body_chunk, "(some huge filecontent");

	request_string =
		"3456789 ABOBA)\r\n"
		"--aboba123--\r\n";

	status = parser.parse_chunk(request_string);
	std::cout << "Status: " << status.msg() << std::endl;
	EXPECT_EQ(request.body_chunk, " pizdec nahuy 123456789 ABOBA)");
	EXPECT_EQ(request.filename, "example.txt");
	EXPECT_EQ(request.mime_type, ".txt");
	EXPECT_EQ(request.content_length, 148);
}