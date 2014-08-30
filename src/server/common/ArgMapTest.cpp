/*
 *  Created on: 2014-08-29
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <server/common/ArgMap.h>
#include <iostream>

TEST(ArgMapTest, BasicTest) {
  using namespace sail;
  const int argc = 9;
  const char *argv[argc] = {"nameOfThisProgram, which is not an argument", "--slice", "10", "40", "filename.txt", "filename2.txt", "--out", "filename3.txt", "--help"};

  ArgMap map(argc, argv);
  map.registerHelpInfo("Some help info for the BasicTest gtest test program.");
  map.registerKeyword("--slice", 2, 2, "Slices a subset of the data.");
  map.registerKeyword("--out", 1, 1, "Specifies the name of the file to output.");
  map.registerKeyword("--swap", 2, 3, "Some other command not tested here...");

  EXPECT_FALSE(map.hasKeyword("rulle"));
  EXPECT_TRUE(map.hasKeyword("--slice"));
  EXPECT_EQ(map.argsAfterKeyword("--slice")[0]->value(), "10");
  EXPECT_TRUE(map.argsAfterKeyword("--slice")[0]->wasRead());
  EXPECT_EQ(map.argsAfterKeyword("--slice")[1]->value(), "40");
  EXPECT_TRUE(map.hasKeyword("--out"));
  EXPECT_EQ(map.argsAfterKeyword("--out")[0]->value(), "filename3.txt");

  Array<ArgMap::Entry*> remain = map.unreadArgs();
  EXPECT_EQ(remain.size(), 2);
  EXPECT_EQ(remain[0]->value(), "filename.txt");
  EXPECT_EQ(remain[1]->value(), "filename2.txt");
  EXPECT_TRUE(map.unreadArgs().empty());
}


