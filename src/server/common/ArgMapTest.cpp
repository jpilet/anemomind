/*
 *  Created on: 2014-08-29
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <server/common/ArgMap.h>
#include <iostream>

using namespace sail;

TEST(ArgMapTest, BasicTest) {
  const int argc = 9;
  const char *argv[argc] = {"nameOfThisProgram, which is not an argument", "--slice", "10", "40", "filename.txt", "filename2.txt", "--out", "filename3.txt", "--help"};

  ArgMap map;
  map.setHelpInfo("Some help info for the BasicTest gtest test program.");
  map.registerOption("--slice", "Slices a subset of the data.").setArgCount(2);
  map.registerOption("--out", "Specifies the name of the file to output.").setArgCount(1);
  map.registerOption("--swap", "Some other command not tested here...").setMinArgCount(1).setMaxArgCount(2);
  map.registerOption("--rulle", "Use short form of Rudolf").setArgCount(0);
  EXPECT_TRUE(map.parse(argc, argv) != ArgMap::Error);

  EXPECT_TRUE(map.hasRegisteredOption("--slice"));
  EXPECT_FALSE(map.optionProvided("--rulle"));
  EXPECT_TRUE(map.optionProvided("--slice"));
  EXPECT_EQ(map.optionArgs("--slice").size(), 2);
  EXPECT_EQ(map.optionArgs("--slice")[0]->value(), "10");
  EXPECT_TRUE(map.optionArgs("--slice")[0]->wasRead());
  EXPECT_EQ(map.optionArgs("--slice")[1]->value(), "40");
  EXPECT_TRUE(map.optionProvided("--out"));
  EXPECT_EQ(map.optionArgs("--out")[0]->value(), "filename3.txt");

  Array<ArgMap::Arg*> remain = map.freeArgs();
  EXPECT_EQ(remain.size(), 2);
  EXPECT_EQ(remain[0]->value(), "filename.txt");
  EXPECT_EQ(remain[1]->value(), "filename2.txt");
  EXPECT_TRUE(map.freeArgs().empty());
}

TEST(ArgMapTest, Help) {
  ArgMap amap;
  const int argc = 2;
  const char *argv[2] = {"programName", "--help"};
  EXPECT_TRUE(amap.parse(argc, argv) != ArgMap::Error);
  EXPECT_TRUE(amap.optionProvided("--help"));
}

namespace {
  ArgMap makeReqMap() {
    ArgMap map;
    map.registerOption("--filename", "Filename, must be specified").setRequired().setMinArgCount(1);
    return map;
  }
}

TEST(ArgMapTest, Required1) {
  const int argc = 1;
  const char *argv[argc] = {"progname"};
  EXPECT_EQ(makeReqMap().parse(argc, argv), ArgMap::Error);
}

TEST(ArgMapTest, Required2) {
  const int argc = 2;
  const char *argv[argc] = {"progname", "--filename"};
  EXPECT_EQ(makeReqMap().parse(argc, argv), ArgMap::Error);
}

TEST(ArgMapTest, Required3) {
  const int argc = 3;
  const char *argv[argc] = {"progname", "--filename", "data.txt"};
  ArgMap m = makeReqMap();
  EXPECT_EQ(m.parse(argc, argv), ArgMap::Continue);
  EXPECT_EQ(m.optionArgs("--filename").size(), 1);
  EXPECT_EQ(m.optionArgs("--filename")[0]->value(), "data.txt");
  EXPECT_TRUE(m.freeArgs().empty());
}

TEST(ArgMapTest, Unique) {
  const int argc = 3;
  const char *argv[argc] = {"progname", "--filename", "--filename"};
  ArgMap m;
  m.registerOption("--filename", "Provides a filename").setUnique();
  EXPECT_EQ(m.parse(argc, argv), ArgMap::Error);
}

TEST(ArgMapTest, Unique2) {
  const int argc = 2;
  const char *argv[argc] = {"progname", "--filename"};
  ArgMap m;
  m.registerOption("--filename", "Provides a filename").setUnique();
  EXPECT_EQ(m.parse(argc, argv), ArgMap::Continue);
}

TEST(ArgMapTest, CallBack) {
  const char *argv[] = {"progname", "-s", "string", "-i", "2600", "-d", "3.23"};
  ArgMap m;
  std::string s = "toto";
  int i = 0;
  double d = 7.0;
  m.registerOption("-s", "Provide a string").store(&s);
  m.registerOption("-i", "Provide an int").store(&i);
  m.registerOption("-d", "Provide a double").store(&d);
  EXPECT_EQ(m.parse(sizeof(argv)/sizeof(argv[0]), argv), ArgMap::Continue);
  EXPECT_EQ(s, "string");
  EXPECT_EQ(i, 2600);
  EXPECT_NEAR(3.23, d, 1e-6);
}
