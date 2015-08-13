/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <server/common/ArrayIO.h>
#include <server/nautical/Sessions.h>
#include <server/common/string.h>

using namespace sail;
using namespace sail::Sessions;

TEST(SessionsTest, TestIrene) {
  auto sessions = segment(getTestdataNavs(), Duration<double>::seconds(12));
  std::cout << EXPR_AND_VAL_AS_STRING(sessions) << std::endl;
}



