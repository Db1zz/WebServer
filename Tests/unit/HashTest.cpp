#include <gtest/gtest.h>
#include <iostream>
#include <set>
#include "Utilities/hash.hpp"

TEST(HashGeneratorTest, GenerateRandomBasicTest) {
	std::string hash = HashGenerator::generate(12);
		
	std::cout << "Generated 12-char hash: " << hash << std::endl;
	EXPECT_EQ(hash.length(), 12);
	EXPECT_FALSE(hash.empty());
}

TEST(HashGeneratorTest, GenerateRandomLengthTest) {
	std::string hash16 = HashGenerator::generate(16);
	std::string hash32 = HashGenerator::generate(32);
	std::string hash8 = HashGenerator::generate(8);
		
	std::cout << "Generated 16-char hash: " << hash16 << std::endl;
	std::cout << "Generated 32-char hash: " << hash32 << std::endl;
	std::cout << "Generated 8-char hash: " << hash8 << std::endl;
		
	EXPECT_EQ(hash16.length(), 16);
	EXPECT_EQ(hash32.length(), 32);
	EXPECT_EQ(hash8.length(), 8);
}

TEST(HashGeneratorTest, GenerateRandomCharacterSetTest) {
	std::string hash = HashGenerator::generate(100); 
	std::cout << "Generated 100-char hash: " << hash << std::endl;
	const std::string validChars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
		
	for (char c : hash) {
		EXPECT_NE(validChars.find(c), std::string::npos) 
			<< "Invalid character found: " << c;
	}
}

TEST(HashGeneratorTest, GenerateRandomUniquenessTest) {
	std::set<std::string> hashes;
	const int numHashes = 10;
	const int hashLength = 20;
		
	std::cout << "Testing uniqueness with " << numHashes << " hashes:" << std::endl;
		
	for (int i = 0; i < numHashes; ++i) {
		std::string hash = HashGenerator::generate(hashLength);
		std::cout << "Hash " << (i + 1) << ": " << hash << std::endl;
		EXPECT_EQ(hashes.find(hash), hashes.end()) 
			<< "Duplicate hash found: " << hash;
		
		hashes.insert(hash);
		EXPECT_EQ(hash.length(), hashLength);
	}
	EXPECT_EQ(hashes.size(), numHashes);
}

TEST(HashGeneratorTest, GenerateRandomZeroLengthTest) {
	std::string hash = HashGenerator::generate(0);
	std::cout << "Generated 0-char hash: '" << hash << "'" << std::endl;
		
	EXPECT_EQ(hash.length(), 0);
	EXPECT_TRUE(hash.empty());
}