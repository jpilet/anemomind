/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/common/CsvParser.h>
#include <gtest/gtest.h>
#include <sstream>

using namespace sail;

TEST(CsvParserTest, NumbersAndStrings) {
  std::stringstream ss;
  ss << "a,b,c\n\n\n1,2,3\n";
  MDArray<std::string, 2> results = parseCsv(&ss);
  EXPECT_EQ(results.cols(), 3);
  EXPECT_EQ(results.rows(), 2);
  EXPECT_EQ(results(0, 0), "a");
  EXPECT_EQ(results(0, 1), "b");
  EXPECT_EQ(results(0, 2), "c");
  EXPECT_EQ(results(1, 0), "1");
  EXPECT_EQ(results(1, 1), "2");
  EXPECT_EQ(results(1, 2), "3");
}


