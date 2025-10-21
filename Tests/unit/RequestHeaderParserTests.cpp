#include <gtest/gtest.h>

#include "RequestParser/RequestHeaderParser.hpp"

TEST(ClassRequestHeaderParser, ParseSimpleHeaderTest) {
	const std::string header =
		"GET /index.html HTTP/1.1\r\n"
		"Host: 127.0.0.1:8080\r\n"
		"User-Agent: SimpleClient/1.0\r\n"
		"\r\n";

	Status status;
	ServerLogger server_logger("/app/Logs");

	server_logger.init();
	RequestHeaderParser parser(&server_logger);
	size_t start = 0;
	status = parser.feed(header, start);
	EXPECT_EQ(status.error(), OK);

	t_request request;
	status = parser.apply(request);
	EXPECT_EQ(status.error(), OK);
	EXPECT_EQ(request.method, "GET");
	EXPECT_EQ(request.host, "127.0.0.1:8080");
	EXPECT_EQ(request.protocol_version, "HTTP/1.1");
}

TEST(ClassRequestHeaderParser, ParseHeaderWithAcceptParams) {
	const std::string header =
		"GET /index.html HTTP/1.1\r\n"
		"Host: 127.0.0.1:8080\r\n"
		"User-Agent: SimpleClient/1.0\r\n"
		"Accept: application/xml;q=0.9, */*;q=0.8\r\n"
		"\r\n";

	Status status;
	ServerLogger server_logger("/app/Logs");

	server_logger.init();
	RequestHeaderParser parser(&server_logger);
	size_t start = 0;
	status = parser.feed(header, start);
	EXPECT_EQ(status.error(), OK);

	t_request request;
	status = parser.apply(request);
	EXPECT_EQ(status.error(), OK);
	EXPECT_EQ(request.method, "GET");
	EXPECT_EQ(request.host, "127.0.0.1:8080");
	EXPECT_EQ(request.protocol_version, "HTTP/1.1");
	EXPECT_EQ(request.accept, "application/xml;q=0.9, */*;q=0.8");
}

TEST(ClassRequestHeaderParser, ParseHeaderWithInvalidAcceptParams) {
	const std::string header =
		"GET /index.html HTTP/1.1\r\n"
		"Host: 127.0.0.1:8080\r\n"
		"User-Agent: SimpleClient/1.0\r\n"
		"Accept: application/xml;q=0.9, */*;q=\r\n"
		"\r\n";

	Status status;
	ServerLogger server_logger("/app/Logs");

	server_logger.init();
	RequestHeaderParser parser(&server_logger);
	size_t start = 0;
	status = parser.feed(header, start);
	EXPECT_EQ(status.error(), OK);

	t_request request;
	status = parser.apply(request);
	EXPECT_EQ(status.error(), BadRequest);
	EXPECT_EQ(request.method, "GET");
	EXPECT_EQ(request.host, "127.0.0.1:8080");
	EXPECT_EQ(request.protocol_version, "HTTP/1.1");
	EXPECT_EQ(request.accept, "");
}

TEST(ClassRequestHeaderParser, ParseHeaderWithInvalidAcceptParams2) {
	const std::string header =
		"GET /index.html HTTP/1.1\r\n"
		"Host: 127.0.0.1:8080\r\n"
		"User-Agent: SimpleClient/1.0\r\n"
		"Accept: ;q=0.9, */*;q=0.9\r\n"
		"\r\n";

	Status status;
	ServerLogger server_logger("/app/Logs");

	server_logger.init();
	RequestHeaderParser parser(&server_logger);
	size_t start = 0;
	status = parser.feed(header, start);
	EXPECT_EQ(status.error(), OK);

	t_request request;
	status = parser.apply(request);
	EXPECT_EQ(status.error(), BadRequest);
	EXPECT_EQ(request.method, "GET");
	EXPECT_EQ(request.host, "127.0.0.1:8080");
	EXPECT_EQ(request.protocol_version, "HTTP/1.1");
	EXPECT_EQ(request.accept, "");
}

TEST(ClassRequestHeaderParser, ParseHeaderWithMultipleAcceptParams) {
	const std::string header =
		"GET /index.html HTTP/1.1\r\n"
		"Host: 127.0.0.1:8080\r\n"
		"User-Agent: SimpleClient/1.0\r\n"
		"Accept: application/xml;q=0.9;level=1, */*;q=0.5\r\n"
		"\r\n";

	Status status;
	ServerLogger server_logger("/app/Logs");

	server_logger.init();
	RequestHeaderParser parser(&server_logger);
	size_t start = 0;
	status = parser.feed(header, start);
	EXPECT_EQ(status.error(), OK);

	t_request request;
	status = parser.apply(request);
	EXPECT_EQ(status.error(), OK);
	EXPECT_EQ(request.method, "GET");
	EXPECT_EQ(request.host, "127.0.0.1:8080");
	EXPECT_EQ(request.protocol_version, "HTTP/1.1");
	EXPECT_EQ(request.accept, "application/xml;q=0.9;level=1, */*;q=0.5");
}

TEST(ClassRequestHeaderParser, ParseHeaderWithMultipleQAcceptParams) {
	const std::string header =
		"GET /index.html HTTP/1.1\r\n"
		"Host: 127.0.0.1:8080\r\n"
		"User-Agent: SimpleClient/1.0\r\n"
		"Accept: application/xml;q=0.9;q=0.8;level=1, */*;q=0.5\r\n"
		"\r\n";

	Status status;
	ServerLogger server_logger("/app/Logs");

	server_logger.init();
	RequestHeaderParser parser(&server_logger);
	size_t start = 0;
	status = parser.feed(header, start);
	EXPECT_EQ(status.error(), OK);

	t_request request;
	status = parser.apply(request);
	EXPECT_EQ(status.error(), OK);
	EXPECT_EQ(request.method, "GET");
	EXPECT_EQ(request.host, "127.0.0.1:8080");
	EXPECT_EQ(request.protocol_version, "HTTP/1.1");
	EXPECT_EQ(request.accept, "application/xml;q=0.9;q=0.8;level=1, */*;q=0.5");
}

TEST(ClassRequestHeaderParser, ParseHeaderWithAllAcceptParams) {
	const std::string header =
		"GET /index.html HTTP/1.1\r\n"
		"Host: 127.0.0.1:8080\r\n"
		"User-Agent: SimpleClient/1.0\r\n"
		"Accept: */*;q=0.5\r\n"
		"\r\n";

	Status status;
	ServerLogger server_logger("/app/Logs");

	server_logger.init();
	RequestHeaderParser parser(&server_logger);
	size_t start = 0;
	status = parser.feed(header, start);
	EXPECT_EQ(status.error(), OK);

	t_request request;
	status = parser.apply(request);
	EXPECT_EQ(status.error(), OK);
	EXPECT_EQ(request.method, "GET");
	EXPECT_EQ(request.host, "127.0.0.1:8080");
	EXPECT_EQ(request.protocol_version, "HTTP/1.1");
	EXPECT_EQ(request.accept, "*/*;q=0.5");
}

TEST(ClassRequestHeaderParser, ParseHeaderWithContentLength) {
	const std::string header =
		"GET /index.html HTTP/1.1\r\n"
		"Host: 127.0.0.1:8080\r\n"
		"User-Agent: SimpleClient/1.0\r\n"
		"Accept: */*;q=0.5\r\n"
		"Content-Length: 1234\r\n"
		"\r\n";

	Status status;
	ServerLogger server_logger("/app/Logs");

	server_logger.init();
	RequestHeaderParser parser(&server_logger);
	size_t start = 0;
	status = parser.feed(header, start);
	EXPECT_EQ(status.error(), OK);

	t_request request;
	status = parser.apply(request);
	EXPECT_EQ(status.error(), OK);
	EXPECT_EQ(request.method, "GET");
	EXPECT_EQ(request.host, "127.0.0.1:8080");
	EXPECT_EQ(request.protocol_version, "HTTP/1.1");
	EXPECT_EQ(request.accept, "*/*;q=0.5");
	EXPECT_EQ(request.content_length, 1234);
}

TEST(ClassRequestHeaderParser, ParseHeaderWithEmptyContentLength) {
	const std::string header =
		"GET /index.html HTTP/1.1\r\n"
		"Host: 127.0.0.1:8080\r\n"
		"User-Agent: SimpleClient/1.0\r\n"
		"Accept: */*;q=0.5\r\n"
		"Content-Length: \r\n"
		"\r\n";

	Status status;
	ServerLogger server_logger("/app/Logs");

	server_logger.init();
	RequestHeaderParser parser(&server_logger);
	size_t start = 0;
	status = parser.feed(header, start);
	EXPECT_EQ(status.error(), OK);

	t_request request;
	status = parser.apply(request);
	EXPECT_EQ(status.error(), BadRequest);
	EXPECT_EQ(request.method, "GET");
	EXPECT_EQ(request.host, "127.0.0.1:8080");
	EXPECT_EQ(request.protocol_version, "HTTP/1.1");
	EXPECT_EQ(request.accept, "*/*;q=0.5");
	EXPECT_EQ(request.content_length, 0);
}

TEST(ClassRequestHeaderParser, ParseHeaderWithTextInContentLength) {
	const std::string header =
		"GET /index.html HTTP/1.1\r\n"
		"Host: 127.0.0.1:8080\r\n"
		"User-Agent: SimpleClient/1.0\r\n"
		"Accept: */*;q=0.5\r\n"
		"Content-Length: asdfasdfasdf\r\n"
		"\r\n";

	Status status;
	ServerLogger server_logger("/app/Logs");

	server_logger.init();
	RequestHeaderParser parser(&server_logger);
	size_t start = 0;
	status = parser.feed(header, start);
	EXPECT_EQ(status.error(), OK);

	t_request request;
	status = parser.apply(request);
	EXPECT_EQ(status.error(), BadRequest);
	EXPECT_EQ(request.method, "GET");
	EXPECT_EQ(request.host, "127.0.0.1:8080");
	EXPECT_EQ(request.protocol_version, "HTTP/1.1");
	EXPECT_EQ(request.accept, "*/*;q=0.5");
	EXPECT_EQ(request.content_length, 0);
}

TEST(ClassRequestHeaderParser, ParseHeaderWithDoubleInContentLength) {
	const std::string header =
		"GET /index.html HTTP/1.1\r\n"
		"Host: 127.0.0.1:8080\r\n"
		"User-Agent: SimpleClient/1.0\r\n"
		"Accept: */*;q=0.5\r\n"
		"Content-Length: 1.0\r\n"
		"\r\n";

	Status status;
	ServerLogger server_logger("/app/Logs");

	server_logger.init();
	RequestHeaderParser parser(&server_logger);
	size_t start = 0;
	status = parser.feed(header, start);
	EXPECT_EQ(status.error(), OK);

	t_request request;
	status = parser.apply(request);
	EXPECT_EQ(status.error(), BadRequest);
	EXPECT_EQ(request.method, "GET");
	EXPECT_EQ(request.host, "127.0.0.1:8080");
	EXPECT_EQ(request.protocol_version, "HTTP/1.1");
	EXPECT_EQ(request.accept, "*/*;q=0.5");
	EXPECT_EQ(request.content_length, 0);
}

TEST(ClassRequestHeaderParser, ParseHeaderWithZeroInFrontOfANumberInContentLength) {
	const std::string header =
		"GET /index.html HTTP/1.1\r\n"
		"Host: 127.0.0.1:8080\r\n"
		"User-Agent: SimpleClient/1.0\r\n"
		"Accept: */*;q=0.5\r\n"
		"Content-Length: 010\r\n"
		"\r\n";

	Status status;
	ServerLogger server_logger("/app/Logs");

	server_logger.init();
	RequestHeaderParser parser(&server_logger);
	size_t start = 0;
	status = parser.feed(header, start);
	EXPECT_EQ(status.error(), OK);

	t_request request;
	status = parser.apply(request);
	EXPECT_EQ(status.error(), BadRequest);
	EXPECT_EQ(request.method, "GET");
	EXPECT_EQ(request.host, "127.0.0.1:8080");
	EXPECT_EQ(request.protocol_version, "HTTP/1.1");
	EXPECT_EQ(request.accept, "*/*;q=0.5");
	EXPECT_EQ(request.content_length, 0);
}

TEST(ClassRequestHeaderParser, ParseHeaderWithKeepAliveInConnection) {
	const std::string header =
		"GET /index.html HTTP/1.1\r\n"
		"Host: 127.0.0.1:8080\r\n"
		"User-Agent: SimpleClient/1.0\r\n"
		"Accept: */*;q=0.5\r\n"
		"Content-Length: 10\r\n"
		"Connection: keep-alive\r\n"
		"\r\n";

	Status status;
	ServerLogger server_logger("/app/Logs");

	server_logger.init();
	RequestHeaderParser parser(&server_logger);
	size_t start = 0;
	status = parser.feed(header, start);
	EXPECT_EQ(status.error(), OK);

	t_request request;
	status = parser.apply(request);
	EXPECT_EQ(status.error(), OK);
	EXPECT_EQ(request.method, "GET");
	EXPECT_EQ(request.host, "127.0.0.1:8080");
	EXPECT_EQ(request.protocol_version, "HTTP/1.1");
	EXPECT_EQ(request.accept, "*/*;q=0.5");
	EXPECT_EQ(request.content_length, 10);
	EXPECT_EQ(request.connection, "keep-alive");
}

TEST(ClassRequestHeaderParser, ParseHeaderWithInvalidValueInConnection) {
	const std::string header =
		"GET /index.html HTTP/1.1\r\n"
		"Host: 127.0.0.1:8080\r\n"
		"User-Agent: SimpleClient/1.0\r\n"
		"Accept: */*;q=0.5\r\n"
		"Content-Length: 10\r\n"
		"Connection: fasdfasdfasdf\r\n"
		"\r\n";

	Status status;
	ServerLogger server_logger("/app/Logs");

	server_logger.init();
	RequestHeaderParser parser(&server_logger);
	size_t start = 0;
	status = parser.feed(header, start);
	EXPECT_EQ(status.error(), OK);

	t_request request;
	status = parser.apply(request);
	EXPECT_EQ(status.error(), BadRequest);
	EXPECT_EQ(request.method, "GET");
	EXPECT_EQ(request.host, "127.0.0.1:8080");
	EXPECT_EQ(request.protocol_version, "HTTP/1.1");
	EXPECT_EQ(request.accept, "*/*;q=0.5");
	EXPECT_EQ(request.content_length, 10);
}

TEST(ClassRequestHeaderParser, ParseHeaderWithUnknownField) {
	const std::string header =
		"GET /index.html HTTP/1.1\r\n"
		"Host: 127.0.0.1:8080\r\n"
		"User-Agent: SimpleClient/1.0\r\n"
		"Accept: */*;q=0.5\r\n"
		"Content-Length: 10\r\n"
		"Aboba: :wisetree:\r\n"
		"\r\n";

	Status status;
	ServerLogger server_logger("/app/Logs");

	server_logger.init();
	RequestHeaderParser parser(&server_logger);
	size_t start = 0;
	status = parser.feed(header, start);
	EXPECT_EQ(status.error(), OK);

	t_request request;
	status = parser.apply(request);
	EXPECT_EQ(status.error(), OK);
	EXPECT_EQ(request.method, "GET");
	EXPECT_EQ(request.host, "127.0.0.1:8080");
	EXPECT_EQ(request.protocol_version, "HTTP/1.1");
	EXPECT_EQ(request.accept, "*/*;q=0.5");
	EXPECT_EQ(request.content_length, 10);
}

TEST(ClassRequestHeaderParser, ParseHeaderWithoutHeaderEnd) {
	const std::string header =
		"GET /index.html HTTP/1.1\r\n"
		"Host: 127.0.0.1:8080\r\n"
		"User-Agent: SimpleClient/1.0\r\n"
		"Accept: */*;q=0.5\r\n"
		"Content-Length: 10\r\n"
		"Aboba: :wisetree:";

	Status status;
	ServerLogger server_logger("/app/Logs");

	server_logger.init();
	RequestHeaderParser parser(&server_logger);
	size_t start = 0;
	status = parser.feed(header, start);
	EXPECT_EQ(status.error(), DataIsNotReady);

	t_request request;
	status = parser.apply(request);
	EXPECT_EQ(status.error(), DataIsNotReady);
}

TEST(ClassRequestHeaderParser, ParseWithoutHost) {
	const std::string header =
		"GET /index.html HTTP/1.1\r\n"
		"User-Agent: SimpleClient/1.0\r\n"
		"Accept: */*;q=0.5\r\n"
		"Content-Length: 10\r\n"
		"Aboba: :wisetree:";

	Status status;
	ServerLogger server_logger("/app/Logs");

	server_logger.init();
	RequestHeaderParser parser(&server_logger);
	size_t start = 0;
	status = parser.feed(header, start);
	EXPECT_EQ(status.error(), DataIsNotReady);

	t_request request;
	status = parser.apply(request);
	EXPECT_EQ(status.error(), DataIsNotReady);
}

TEST(ClassRequestHeaderParser, ParseWithoutRequestLine) {
	const std::string header =
		"User-Agent: SimpleClient/1.0\r\n"
		"Accept: */*;q=0.5\r\n"
		"Content-Length: 10\r\n"
		"Aboba: :wisetree:\r\n"
		"\r\n";

	Status status;
	ServerLogger server_logger("/app/Logs");

	server_logger.init();
	RequestHeaderParser parser(&server_logger);
	size_t start = 0;
	status = parser.feed(header, start);
	EXPECT_EQ(status.error(), OK);

	t_request request;
	status = parser.apply(request);
	EXPECT_EQ(status.error(), BadRequest);
}

TEST(ClassRequestHeaderParser, ParseWithoutAnyField) {
	const std::string header =
		"\r\n\r\n";

	Status status;
	ServerLogger server_logger("/app/Logs");

	server_logger.init();
	RequestHeaderParser parser(&server_logger);
	size_t start = 0;
	status = parser.feed(header, start);
	EXPECT_EQ(status.error(), OK);

	t_request request;
	status = parser.apply(request);
	EXPECT_EQ(status.error(), BadRequest);
}

TEST(ClassRequestHeaderParser, ParseHeaderWithHexEncodedName) {
	const std::string header =
		"GET /%D0%B0%D0%B1%D0%BE%D0%B1%D0%B0.html HTTP/1.1\r\n"
		"Host: 127.0.0.1:8080\r\n"
		"User-Agent: SimpleClient/1.0\r\n"
		"Accept: */*;q=0.5\r\n"
		"Content-Length: 10\r\n"
		"\r\n";

	Status status;
	ServerLogger server_logger("/app/Logs");

	server_logger.init();
	RequestHeaderParser parser(&server_logger);
	size_t start = 0;
	status = parser.feed(header, start);
	EXPECT_EQ(status.error(), OK);

	t_request request;
	status = parser.apply(request);
	EXPECT_EQ(request.filename, "абоба.html");
	EXPECT_EQ(status.error(), OK);
}

TEST(ClassRequestHeaderParser, ParseHeaderWithInvalidHexEncodedName) {
	const std::string header =
		"GET /%D0%.html HTTP/1.1\r\n"
		"Host: 127.0.0.1:8080\r\n"
		"User-Agent: SimpleClient/1.0\r\n"
		"Accept: */*;q=0.5\r\n"
		"Content-Length: 10\r\n"
		"\r\n";

	Status status;
	ServerLogger server_logger("/app/Logs");

	server_logger.init();
	RequestHeaderParser parser(&server_logger);
	size_t start = 0;
	status = parser.feed(header, start);
	EXPECT_EQ(status.error(), OK);

	t_request request;
	status = parser.apply(request);
	EXPECT_EQ(status.error(), BadRequest);
}

TEST(ClassRequestHeaderParser, ParseHeaderWithQueries) {
	const std::string header =
		"GET /aboba.html?query=parameter&anotherquery=anotherparameter HTTP/1.1\r\n"
		"Host: 127.0.0.1:8080\r\n"
		"User-Agent: SimpleClient/1.0\r\n"
		"Accept: */*;q=0.5\r\n"
		"Content-Length: 10\r\n"
		"\r\n";

	Status status;
	ServerLogger server_logger("/app/Logs");

	server_logger.init();
	RequestHeaderParser parser(&server_logger);
	size_t start = 0;
	status = parser.feed(header, start);
	EXPECT_EQ(status.error(), OK);

	t_request request;
	status = parser.apply(request);
	EXPECT_EQ(status.error(), OK);
	EXPECT_EQ(request.path_queries[0], "query=parameter");
	EXPECT_EQ(request.path_queries[1], "anotherquery=anotherparameter");
}
