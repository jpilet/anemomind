/*
 *  Created on: 2014-03-27
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <server/common/TestEnv.h>

using namespace sail;

TEST(TestEnvTest, TestThisPath) {
  Poco::Path thisPath(__FILE__);

  TestEnv env;

  EXPECT_TRUE(env.root().isDirectory());
  EXPECT_TRUE(env.datasets().isDirectory());

  Poco::Path b = env.root();
  b.pushDirectory("src");
  b.pushDirectory("server");
  b.pushDirectory("common");
  b.setFileName("TestEnvTest.cpp");
  EXPECT_EQ(thisPath.toString(), b.toString());
}



