/*
 *  Created on: 2014-08-29
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <server/common/ArgMap.h>
#include <iostream>

TEST(ArgMapTest, BasicTest) {
  using namespace sail;
  const int argc = 8;
  const char *argv[argc] = {"nameOfThisProgram", "--slice", "10", "40", "filename.txt", "filename2.txt", "--out", "filename3.txt"};

  ArgMap map(argc, argv);
  EXPECT_FALSE(map.hasArg("rulle"));
  EXPECT_TRUE(map.hasArg("--slice"));
  EXPECT_EQ(map.argsAfter("--slice")[0]->value(), "10");
  EXPECT_TRUE(map.argsAfter("--slice")[0]->wasRead());
  EXPECT_EQ(map.argsAfter("--slice")[1]->value(), "40");
  EXPECT_TRUE(map.hasArg("--out"));
  EXPECT_EQ(map.argsAfter("--out")[0]->value(), "filename3.txt");

  Array<ArgMap::Entry::Ptr> remain = map.unreadArgs();
  EXPECT_EQ(remain.size(), 2);
  EXPECT_EQ(remain[0]->value(), "filename.txt");
  EXPECT_EQ(remain[1]->value(), "filename2.txt");
  EXPECT_TRUE(map.unreadArgs().empty());
}


