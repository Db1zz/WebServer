// tests/ServerRequestParserTests.cpp

#include <gtest/gtest.h>

#include <climits>
#include <iostream>
#include <string>

#include "RequestMultipartBodyParser.hpp"
#include "RequestRawBodyParser.hpp"
#include "ServerRequest.hpp"
#include "ServerRequestParser.hpp"
#include "status.hpp"

TEST(ServerRequestParserTest, ParseCompleteGETRequest) {
	Status status;
	t_request request;
	ServerLogger logger("/app/Logs");
	logger.init();
	ServerRequestParser parser(&request, &logger);

	std::string request_string =
		"GET /aboba.html HTTP/1.1\r\n"
		"Host: 0.0.0.0\r\n"
		"User-Agent: Mozilla/5.0 (AbobaChips; Quantum Computer X 10.15; rv:142.0) Gecko/1337 "
		"Firefox/142.0\r\n"
		"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
		"Accept-Language: en-GB,en;q=0.5\r\n"
		"Accept-Encoding: gzip, deflate, br, zstd\r\n"
		"Connection: keep-alive\r\n"
		"Priority: u=0, i\r\n"
		"\r\n";

	status = parser.feed(request_string);
	EXPECT_TRUE((bool) status);
	EXPECT_EQ(request.method, "GET");
	EXPECT_EQ(request.protocol_version, "HTTP/1.1");
	EXPECT_EQ(request.filename, "aboba.html");
	EXPECT_EQ(request.host, "0.0.0.0");
	EXPECT_EQ(request.accept, "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
	EXPECT_EQ(request.connection, "keep-alive");
}

TEST(ServerRequestParserTest, ParseIncompleteGETRequest) {
	Status status;
	ServerLogger logger("/app/Logs");
	t_request request;
	ServerRequestParser parser(&request, &logger);

	std::string request_string =
		"GET /aboba.html HTTP/1.1\r\n"
		"Host: 0.0.0.0\r\n"
		"User-Agent: Mozilla/5.0 (AbobaChips; Quantum Computer X 10.15; rv:142.0) Gecko/1337 "
		"Firefox/142.0\r\n"
		"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
		"Accept-Language: en-GB,en;q=0.5\r\n"
		"Accept-Encoding: gzip, deflate, br, zstd\r\n"
		"Connection: keep-alive\r\n"
		"Priority: u=0, i\r\n";

	status = parser.feed(request_string);
	EXPECT_EQ(status.error(), DataIsNotReady);
}

TEST(ServerRequestParserTest, ParseCompletePostRequest) {
	Status status;
	ServerLogger logger("/app/Logs");
	t_request request;
	ServerRequestParser parser(&request, &logger);

	std::string request_string =
		"POST /upload HTTP/1.1\r\n"
		"Host: 0.0.0.0\r\n"
		"Content-Type: multipart/form-data; boundary=aboba123\r\n"
		"Content-Length: 146\r\n"
		"\r\n"
		"--aboba123\r\n"
		"Content-Disposition: form-data; name=\"file\"; filename=\"example.txt\"\r\n"
		"Content-Type: text/plain\r\n"
		"\r\n"
		"(file contents here)\r\n"
		"--aboba123--\r\n";

	status = parser.feed(request_string);
	EXPECT_TRUE((bool) status);
	EXPECT_FALSE(request.content_data.empty());
	EXPECT_EQ(request.content_data.front().name, "file");
	EXPECT_EQ(request.content_data.front().filename, "example.txt");
	EXPECT_EQ(request.content_data.front().data, "(file contents here)");
	EXPECT_EQ(request.content_length, 146u);
	EXPECT_EQ(request.host, "0.0.0.0");
}

TEST(ServerRequestParserTest, ParsePartitionedPostRequest) {
	Status status;
	ServerLogger logger("/app/Logs");
	t_request request;
	ServerRequestParser parser(&request, &logger);

	std::string request_string =
		"POST /upload HTTP/1.1\r\n"
		"Host: 0.0.0.0\r\n"
		"Content-Type: multipart/form-data; boundary=aboba123\r\n"
		"Content-Length: 148\r\n"
		"\r\n"
		"--aboba123\r\n"
		"Content-Disposition: form-data; name=\"file\"; filename=\"example.txt\"\r\n"
		"Content-Type: text/plain\r\n"
		"\r\n"
		"(some huge filecontent pizdec nahuy 12";

	status = parser.feed(request_string);
	EXPECT_EQ(request.content_data.front().data, "(some huge filecontent");
	EXPECT_FALSE(request.content_data.front().is_finished);

	request_string =
		"3456789 ABOBA)\r\n"
		"--aboba123--\r\n";

	status = parser.feed(request_string);
	EXPECT_TRUE((bool) status);
	EXPECT_EQ(request.content_data.front().name, "file");
	EXPECT_EQ(request.content_data.front().filename, "example.txt");
	EXPECT_EQ(request.content_data.front().data,
			  "(some huge filecontent pizdec nahuy 123456789 ABOBA)");
	EXPECT_EQ(request.filename, "upload");
	EXPECT_EQ(request.content_length, 148u);
	EXPECT_TRUE(request.content_data.front().is_finished);
}