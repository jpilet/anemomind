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

  ArgMap map;
  map.setHelpInfo("Some help info for the BasicTest gtest test program.");
  map.registerOption("--slice", "Slices a subset of the data.").argCount(2);
  map.registerOption("--out", "Specifies the name of the file to output.").argCount(1);
  map.registerOption("--swap", "Some other command not tested here...").minArgs(1).maxArgs(2);
  map.registerOption("--rulle", "Use short form of Rudolf").argCount(0);
  map.parse(argc, argv);

  EXPECT_FALSE(map.hasOption("--rulle"));
  EXPECT_TRUE(map.hasOption("--slice"));
  EXPECT_EQ(map.argsAfterOption("--slice").size(), 2);
  EXPECT_EQ(map.argsAfterOption("--slice")[0]->value(), "10");
  EXPECT_TRUE(map.argsAfterOption("--slice")[0]->wasRead());
  EXPECT_EQ(map.argsAfterOption("--slice")[1]->value(), "40");
  EXPECT_TRUE(map.hasOption("--out"));
  EXPECT_EQ(map.argsAfterOption("--out")[0]->value(), "filename3.txt");

  Array<ArgMap::Entry*> remain = map.freeArgs();
  EXPECT_EQ(remain.size(), 2);
  EXPECT_EQ(remain[0]->value(), "filename.txt");
  EXPECT_EQ(remain[1]->value(), "filename2.txt");
  EXPECT_TRUE(map.freeArgs().empty());
}


