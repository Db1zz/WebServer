#include <gtest/gtest.h>

#include <string>

#include "RequestParser/RequestMultipartBodyParser.hpp"
#include "ServerRequest.hpp"

TEST(FeedTests, ParseCorrectBodyWithSingleHeader) {
	Status status;
	std::string body =
		"------WebKitFormBoundary7MA4YWxkTrZu0gW\r\n"
		"Content-Disposition: form-data; name=\"username\"\r\n"
		"\r\n"
		"vasya\r\n"
		"------WebKitFormBoundary7MA4YWxkTrZu0gW--\r\n";
	RequestMultipartParser parser("----WebKitFormBoundary7MA4YWxkTrZu0gW", 143);
	status = parser.feed(body, 0);
	EXPECT_NE(status, BadRequest);
	t_request request;
	parser.apply(request);

	EXPECT_EQ(request.content_data.front().data, "vasya");
	EXPECT_EQ(request.content_data.front().name, "username");
	EXPECT_TRUE(request.content_data.front().is_finished);
}

TEST(FeedTests, ParseMultiplePartsOfASingleRequestBody) {
	Status status;
	RequestMultipartParser parser("----WebKitFormBoundary7MA4YWxkTrZu0gW", 143);
	std::string body = "------WebKitFormBoundary7MA4YWxkTrZu0gW\r\n";
	status = parser.feed(body, 0);
	body = "Content-Disposition:";
	status = parser.feed(body, 0);
	body = " form-data; name=\"username\"\r\n";
	status = parser.feed(body, 0);
	body = "\r\nvasya";
	status = parser.feed(body, 0);
	body = "\r\n------WebKitForm";
	status = parser.feed(body, 0);
	body = "Boundary7MA4YWxkTrZu0gW--\r\n";
	status = parser.feed(body, 0);
	EXPECT_NE(status, BadRequest);
	t_request request;
	parser.apply(request);
	EXPECT_EQ(request.content_data.front().data, "vasya");
	EXPECT_EQ(request.content_data.front().name, "username");
}

TEST(FeedTests, ParseWholeMultipartBody) {
	Status status;
	std::string body =
		"------WebKitFormBoundary7MA4YWxkTrZu0gW\r\n"
		"Content-Disposition: form-data; name=\"username\"\r\n"
		"\r\n"
		"vasya\r\n"
		"------WebKitFormBoundary7MA4YWxkTrZu0gW\r\n"
		"Content-Disposition: form-data; name=\"aboba\"; filename=\"myrare_filename.txt\"\r\n"
		"\r\n"
		"pizdec kontenta\r\n"
		"------WebKitFormBoundary7MA4YWxkTrZu0gW--\r\n";
	RequestMultipartParser parser("----WebKitFormBoundary7MA4YWxkTrZu0gW", 143);
	status = parser.feed(body, 0);
	EXPECT_NE(status, BadRequest);
	t_request request;
	parser.apply(request);
	EXPECT_EQ(request.content_data.front().data, "vasya");
	EXPECT_EQ(request.content_data.front().name, "username");
	EXPECT_TRUE(request.content_data.front().is_finished);
	request.content_data.pop_front();
	EXPECT_EQ(request.content_data.front().data, "pizdec kontenta");
	EXPECT_EQ(request.content_data.front().name, "aboba");
	EXPECT_EQ(request.content_data.front().filename, "myrare_filename.txt");
	EXPECT_TRUE(request.content_data.front().is_finished);
}

TEST(FeedTests, ParseSplittedMultipartBody) {
	Status status;
	RequestMultipartParser parser("----WebKitFormBoundary7MA4YWxkTrZu0gW", 143);
	std::string body = "------WebKitFormBoundary7MA4YWxkTrZu0gW\r\n";
	status = parser.feed(body, 0);
	body = "Content-Disposition: form-";
	status = parser.feed(body, 0);
	body = "data; name=\"username\"\r\n";
	status = parser.feed(body, 0);
	body = "\r\n";
	status = parser.feed(body, 0);
	body = "vasya";
	status = parser.feed(body, 0);
	body = "\r\n";
	status = parser.feed(body, 0);
	body = "------WebKitFormBoundary";
	status = parser.feed(body, 0);
	body = "7MA4YWxkTrZu0gW\r\n";
	status = parser.feed(body, 0);
	body = "Content";
	status = parser.feed(body, 0);
	body = "-Disposition: form-data; name=\"aboba\"; filename=\"myrare";
	status = parser.feed(body, 0);
	body = "_filename.txt\"\r\n";
	status = parser.feed(body, 0);
	body = "\r\n";
	status = parser.feed(body, 0);
	body = "pizdec";
	status = parser.feed(body, 0);
	body = " kontenta\r\n";
	status = parser.feed(body, 0);
	body = "------WebKitFormBoundary";
	status = parser.feed(body, 0);
	body = "7MA4YWxkTrZu0gW--\r\n";
	status = parser.feed(body, 0);
	EXPECT_NE(status, BadRequest);
	t_request request;
	parser.apply(request);
	EXPECT_EQ(request.content_data.front().data, "vasya");
	EXPECT_EQ(request.content_data.front().name, "username");
	EXPECT_TRUE(request.content_data.front().is_finished);
	request.content_data.pop_front();
	EXPECT_EQ(request.content_data.front().data, "pizdec kontenta");
	EXPECT_EQ(request.content_data.front().name, "aboba");
	EXPECT_EQ(request.content_data.front().filename, "myrare_filename.txt");
	EXPECT_TRUE(request.content_data.front().is_finished);
}

TEST(FeedTests, ParseTheWholeRequestByFeedingSingleChar) {
	Status status;
	std::string body =
		"------WebKitFormBoundary7MA4YWxkTrZu0gW\r\n"
		"Content-Disposition: form-data; name=\"username\"\r\n"
		"\r\n"
		"vasya\r\n"
		"------WebKitFormBoundary7MA4YWxkTrZu0gW\r\n"
		"Content-Disposition: form-data; name=\"aboba\"; filename=\"myra\\\"re_file\\\"name.txt\"\r\n"
		"\r\n"
		"pizdec kontenta\r\n"
		"------WebKitFormBoundary7MA4YWxkTrZu0gW--\r\n";
	RequestMultipartParser parser("----WebKitFormBoundary7MA4YWxkTrZu0gW", 281);
	for (size_t i = 0; i < body.size(); ++i) {
		std::string s;
		s = body[i];
		status = parser.feed(s, 0);
		EXPECT_NE(status, BadRequest);
	}
	t_request request;
	parser.apply(request);
	EXPECT_EQ(request.content_data.front().data, "vasya");
	EXPECT_EQ(request.content_data.front().name, "username");
	request.content_data.pop_front();
	EXPECT_EQ(request.content_data.front().data, "pizdec kontenta");
	EXPECT_EQ(request.content_data.front().name, "aboba");
	EXPECT_EQ(request.content_data.front().filename, "myra\"re_file\"name.txt");
}

TEST(FeedTests, ParseBigFileWithTinyOne) {
	Status status;
	std::string body =
		"------WebKitFormBoundary7MA4YWxkTrZu0gW\r\n"
		"Content-Disposition: form-data; name=\"username\"\r\n"
		"\r\n"
		"vasya\r\n"
		"------WebKitFormBoundary7MA4YWxkTrZu0gW\r\n"
		"Content-Disposition: form-data; name=\"aboba\"; filename=\"myrare_filename.txt\"\r\n"
		"\r\n";

	std::string content;
	const size_t megabytes = 1000000 * 100;
	for (size_t i = 0; i < megabytes; ++i) {
		content.push_back('a');
	}

	body.append(content);
	body.append("\r\n------WebKitFormBoundary7MA4YWxkTrZu0gW--\r\n");
	RequestMultipartParser parser("----WebKitFormBoundary7MA4YWxkTrZu0gW", 143);
	status = parser.feed(body, 0);
	EXPECT_NE(status, BadRequest);
	t_request request;
	parser.apply(request);
	EXPECT_EQ(request.content_data.front().data, "vasya");
	EXPECT_EQ(request.content_data.front().name, "username");
	request.content_data.pop_front();
	EXPECT_EQ(request.content_data.front().data, content);
	EXPECT_EQ(request.content_data.front().name, "aboba");
	EXPECT_EQ(request.content_data.front().filename, "myrare_filename.txt");
}
