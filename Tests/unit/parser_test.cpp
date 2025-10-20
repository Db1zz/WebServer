#include <gtest/gtest.h>
// #include "../../Server/Server.hpp"
#include "../../Parser/Parser.hpp"

TEST(ParserValues, Correct) {
	/* Tests to prove that the parsed values are the same */
	Parser parser("../Tests/unit/goodconfig.conf");
	std::vector<t_config> vec = parser.getConfigStruct();
	EXPECT_EQ(vec.size(), 1);
	/* Server name check */
	EXPECT_EQ(vec[0].server_name[0], "webserver.com");
	/* Keepalive_timeout check */
	EXPECT_EQ(vec[0].keepalive_timeout, 42);
	/* Listen check */
	EXPECT_EQ(vec[0].listen[0].host, std::string("0.0.0.0"));
	EXPECT_EQ(vec[0].listen[0].port, 90);
	EXPECT_EQ(vec[0].listen[1].host, std::string("0.0.0.0"));
	EXPECT_EQ(vec[0].listen[1].port, 81);
	EXPECT_EQ(vec[0].listen[2].host, std::string("0.0.0.0"));
	EXPECT_EQ(vec[0].listen[2].port, 80);
	/* Error page check */
	EXPECT_EQ(vec[0].common.errorPage.at(400), std::string("Pages/400.html"));
	EXPECT_EQ(vec[0].common.errorPage.at(404), std::string("Pages/404.html"));
	EXPECT_EQ(vec[0].common.errorPage.at(500), std::string("Pages/500.html"));
	EXPECT_EQ(vec[0].common.errorPage.at(505), std::string("Pages/505.html"));
	EXPECT_EQ(vec[0].common.errorPage.at(406), std::string("Pages/406.html"));
	/* Index check */
	EXPECT_EQ(vec[0].common.index[0], std::string("index.html"));
	/* Auto index */
	EXPECT_TRUE(vec[0].common.auto_index);
	/* Methods */
	EXPECT_TRUE(vec[0].common.methods.getMethod);
	EXPECT_FALSE(vec[0].common.methods.postMethod);
	EXPECT_FALSE(vec[0].common.methods.deleteMethod);
	/* Location / */
	EXPECT_EQ(vec[0].location[0].path, std::string("/"));
	EXPECT_EQ(vec[0].location[0].common.index[0], std::string("index.html"));
	EXPECT_TRUE(vec[0].location[0].common.auto_index);
	EXPECT_TRUE(vec[0].location[0].common.methods.getMethod);
	EXPECT_FALSE(vec[0].location[0].common.methods.postMethod);
	EXPECT_FALSE(vec[0].location[0].common.methods.deleteMethod);
	/* Location /api/files */
	EXPECT_EQ(vec[0].location[1].path, std::string("/api/files"));
	EXPECT_FALSE(vec[0].location[1].common.auto_index);
	EXPECT_TRUE(vec[0].location[1].common.methods.getMethod);
	EXPECT_FALSE(vec[0].location[1].common.methods.postMethod);
	EXPECT_FALSE(vec[0].location[1].common.methods.deleteMethod);
	EXPECT_EQ(vec[0].location[1].common.max_client_body, 42);
	EXPECT_EQ(vec[0].location[1].common.returnCode, 442);
	/* Test unprovided chunked_size -> should be 0 */
	EXPECT_EQ(vec[0].location[1].chunked_size, 0);
	/* Location /Uploads */
	EXPECT_EQ(vec[0].location[2].path, std::string("/Uploads"));
	EXPECT_TRUE(vec[0].location[2].common.auto_index);
	EXPECT_TRUE(vec[0].location[2].common.methods.getMethod);
	EXPECT_TRUE(vec[0].location[2].common.methods.postMethod);
	EXPECT_TRUE(vec[0].location[2].common.methods.deleteMethod);
	EXPECT_EQ(vec[0].location[2].common.returnPath, std::string("google.com"));
	EXPECT_EQ(vec[0].location[2].common.returnCode, 404);
	EXPECT_TRUE(vec[0].location[2].chunked_transfer_encoding);
	EXPECT_EQ(vec[0].location[2].chunked_size, 8192);
	EXPECT_EQ(vec[0].location[2].chunked_threshold, 1024);
}

TEST(BadConfigFile, Error)
{
	EXPECT_EXIT(Parser("../Tests/unit/NoConf"),testing::ExitedWithCode(1), "No file found with the extension '.conf' or no rights to read!\n");
	EXPECT_EXIT(Parser("../Tests/unit/NoRights"),testing::ExitedWithCode(1), "No file found with the extension '.conf' or no rights to read!\n");
}

TEST(BadConfigFile, Duplicate)
{
	EXPECT_THROW(Parser("../Tests/unit/Duplicate.conf"), std::runtime_error);
}

TEST(BadConfigFile, NoServerBlock)
{
	EXPECT_THROW(Parser("../Tests/unit/NoServerBlock.conf"), std::runtime_error);
}

TEST(BadConfigFile, NoPathForLocation)
{
	EXPECT_THROW(Parser("../Tests/unit/NoPathForLocation.conf"), std::runtime_error);
}



int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}