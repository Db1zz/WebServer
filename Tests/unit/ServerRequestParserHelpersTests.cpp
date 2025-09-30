#include <gtest/gtest.h>

#include <string>
#include "RequestParser/ServerRequestParserHelpers.hpp"

TEST(UTF8CharDecoder, Decode1ByteSequence) {
	Status status;

	size_t char_pos = 0;
	std::string encoded_sequence("%D0%B0");

	size_t consumed_chars = 0;
	std::string decoded_sequence;
	
	status = internal_server_request_parser::utf8_char_decoder(encoded_sequence, char_pos, decoded_sequence, consumed_chars);
	
	EXPECT_EQ(decoded_sequence, "а");
}

TEST(UTF8CharDecoder, Decode2ByteSequence) {
	Status status;

	size_t char_pos = 0;
	std::string encoded_sequence("%E2%82%AC");

	size_t consumed_chars = 0;
	std::string decoded_sequence;

	status = internal_server_request_parser::utf8_char_decoder(encoded_sequence, char_pos,
															   decoded_sequence, consumed_chars);

	EXPECT_EQ(decoded_sequence, "€");
}

TEST(UTF8CharDecoder, Decode3ByteSequence) {
	Status status;

	size_t char_pos = 0;
	std::string encoded_sequence("%F0%9F%98%80");

	size_t consumed_chars = 0;
	std::string decoded_sequence;

	status = internal_server_request_parser::utf8_char_decoder(encoded_sequence, char_pos,
															   decoded_sequence, consumed_chars);

	EXPECT_EQ(decoded_sequence, "😀");
}

TEST(UTF8CharDecoder, DecodeInvalidSequenceWithNoContinuationBytes) {
	Status status;

	size_t char_pos = 0;
	std::string encoded_sequence("%D0");

	size_t consumed_chars = 0;
	std::string decoded_sequence;

	status = internal_server_request_parser::utf8_char_decoder(encoded_sequence, char_pos,
															   decoded_sequence, consumed_chars);
	EXPECT_EQ(status.error(), BadRequest);
}

TEST(ParseEncodedFilename, SingleEncodedCharTest) {
	std::string encoded_filename = "UTF-8''%D0%B0.test";
	std::string decoded_filename;

	internal_server_request_parser::parse_encoded_filename(encoded_filename, decoded_filename);
	EXPECT_EQ(decoded_filename, "\xD0\xB0.test");
}

TEST(ParseEncodedFilename, MultipleEncodedCharTest) {
	std::string encoded_filename = "UTF-8''%D0%B0%D0%B1%D0%BE%D0%B1%D0%B0.test";
	std::string decoded_filename;

	internal_server_request_parser::parse_encoded_filename(encoded_filename, decoded_filename);
	EXPECT_EQ(decoded_filename, "\xD0\xB0\xD0\xB1\xD0\xBE\xD0\xB1\xD0\xB0.test");
}

TEST(ParseEncodedFilename, InvalidEncodedCharTest) {
	Status status;
	std::string encoded_filename = "UTF-8''%DAboba.test";
	std::string decoded_filename;

	status = internal_server_request_parser::parse_encoded_filename(encoded_filename, decoded_filename);
	EXPECT_EQ(status.error(), BadRequest);
	EXPECT_EQ(decoded_filename.empty(), true);
}
