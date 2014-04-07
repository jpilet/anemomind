/*
 *  Created on: 2014-04-07
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/common/Env.h>
#include <gtest/gtest.h>
#include <iostream>


TEST(EnvTest, TestNonEmpty) {
  std::string x(sail::Env::SOURCE_DIR);

  // Testing that the source dir is really correct is hard to do in a cross platform fashion.
  std::cout << "The source dir is " << x << std::endl;

  EXPECT_GE(x.length(), 0);
}


