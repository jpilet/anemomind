/*
 *  Created on: 2014-04-07
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/common/Env.h>
#include <gtest/gtest.h>
#include <iostream>
#include <Poco/Path.h>
#include <Poco/File.h>


TEST(EnvTest, TestNonEmpty) {
  std::string x(sail::Env::SOURCE_DIR);

  EXPECT_GT(x.length(), 0);

  Poco::Path envTestPath =
      Poco::Path("src/server/common/EnvTest.cpp").absolute(
          Poco::Path(sail::Env::SOURCE_DIR));
  EXPECT_TRUE(Poco::File(envTestPath).exists());
}
