/*
 *  Created on: 2014-03-27
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <server/common/TestEnv.h>
#include <Poco/Path.h>

using namespace sail;

TEST(TestEnvTest, TestThisPath) {
  Poco::Path thisPath(__FILE__);


  Poco::Path b(TestEnv::SOURCE_DIR);
  b.pushDirectory("server");
  b.pushDirectory("common");
  b.setFileName("TestEnvTest.cpp");
  EXPECT_EQ(thisPath.toString(), b.toString());
}



