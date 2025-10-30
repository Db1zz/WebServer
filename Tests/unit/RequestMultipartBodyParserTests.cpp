// RequestMultipartTests.cpp
#include <gtest/gtest.h>
#include <string>
#include <list>

#include "Server/RequestParser/RequestMultipartBodyParser.hpp" // <- используйте точное имя заголовка в вашем проекте
#include "Server/ServerRequest.hpp"
#include "Logger/ServerLogger.hpp"

TEST(FeedTests, ParseCorrectBodyWithSingleHeader) {
    ServerLogger server_logger("./test_logs");
    server_logger.init();

    Status status;
    std::string body =
        "------WebKitFormBoundary7MA4YWxkTrZu0gW\r\n"
        "Content-Disposition: form-data; name=\"username\"\r\n"
        "\r\n"
        "vasya\r\n"
        "------WebKitFormBoundary7MA4YWxkTrZu0gW--\r\n";
    RequestMultipartParser parser("----WebKitFormBoundary7MA4YWxkTrZu0gW", 143, &server_logger);
    status = parser.feed(body, 0);
    EXPECT_NE(status, BadRequest);
    t_request request;
    parser.apply(request);

    EXPECT_EQ(request.content_data.front().data, "vasya");
    EXPECT_EQ(request.content_data.front().name, "username");
    EXPECT_TRUE(request.content_data.front().is_finished);
}

TEST(FeedTests, ParseMultiplePartsOfASingleRequestBody) {
    ServerLogger server_logger("./test_logs");
    server_logger.init();

    Status status;
    RequestMultipartParser parser("----WebKitFormBoundary7MA4YWxkTrZu0gW", 143, &server_logger);
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
    ServerLogger server_logger("./test_logs");
    server_logger.init();

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
    RequestMultipartParser parser("----WebKitFormBoundary7MA4YWxkTrZu0gW", 143, &server_logger);
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
    ServerLogger server_logger("./test_logs");
    server_logger.init();

    Status status;
    RequestMultipartParser parser("----WebKitFormBoundary7MA4YWxkTrZu0gW", 143, &server_logger);
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
    ServerLogger server_logger("./test_logs");
    server_logger.init();

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
    RequestMultipartParser parser("----WebKitFormBoundary7MA4YWxkTrZu0gW", 281, &server_logger);
    for (size_t i = 0; i < body.size(); ++i) {
        std::string s(1, body[i]);
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
    ServerLogger server_logger("./test_logs");
    server_logger.init();

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
    const size_t five_mib = 5ull * 1024 * 1024;
    content.reserve(five_mib);
    for (size_t i = 0; i < five_mib; ++i) {
        content.push_back('a');
    }

    body.append(content);
    body.append("\r\n------WebKitFormBoundary7MA4YWxkTrZu0gW--\r\n");
    RequestMultipartParser parser("----WebKitFormBoundary7MA4YWxkTrZu0gW", static_cast<int>(body.size()), &server_logger);
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
