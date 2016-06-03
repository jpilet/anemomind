/*
 * StoredTimeTest.cpp
 *
 *  Created on: Jun 3, 2016
 *      Author: jonas
 */

#include <gtest/gtest.h>
#include <server/common/StoredTime.h>
#include <cstdio>
#include <fstream>
#include <chrono>
#include <thread>

using namespace sail;

namespace {
  bool successfulSaveAndLoad(TimeStamp t) {
    std::string filename = "/tmp/saved_time.txt";
    saveTimeStampToFile(filename, t);
    return loadTimeStampFromFile(filename) == t;
  }
}

TEST(StoredTimeTest, SavingAndLoading) {
  std::remove("/tmp/nonexistant_file.txt");
  EXPECT_EQ(TimeStamp(), loadTimeStampFromFile("/tmp/nonexistant_file.txt"));
  EXPECT_TRUE(successfulSaveAndLoad(TimeStamp()));
  EXPECT_TRUE(successfulSaveAndLoad(TimeStamp::UTC(2016, 6, 3, 12, 54, 0)));
}

TEST(StoredTimeTest, Garbage) {
  std::string garbageFilename = "/tmp/garbage_time.txt";
  {
    std::ofstream garbageFile(garbageFilename);
    garbageFile << "Try to parse this into a time! :-P";
  }
  EXPECT_EQ(TimeStamp(), loadTimeStampFromFile(garbageFilename));
}

TEST(StoredTimeTest, TestBasicUsage) {
  std::string filename = "/tmp/stored_time.txt";
  std::remove(filename.c_str());

  auto offset = TimeStamp::UTC(2016, 6, 3, 13, 19, 0);
  auto upper = offset + Duration<double>::minutes(3.0);

  for (int i = 0; i < 5; i++) {
    StoredTime s(filename, offset);
    EXPECT_LE(offset + Duration<double>::milliseconds(i*20), s.now());
    EXPECT_LE(s.now(), upper);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
  }{
    StoredTime s(filename, TimeStamp::UTC(2018, 9, 7, 12, 0, 4));
    EXPECT_LE(offset + Duration<double>::milliseconds(100), s.now());
    EXPECT_LE(s.now(), upper);
  }
}

