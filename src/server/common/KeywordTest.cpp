/*
 * KeywordTest.cpp
 *
 *  Created on: 3 Aug 2017
 *      Author: jonas
 */

#include <gtest/gtest.h>
#include <server/common/Keyword.h>
#include <unordered_map>

using namespace sail;

TEST(KeywordTest, BasicOps) {
  DECLARE_KEYWORD(a);
  DECLARE_KEYWORD(b);

  EXPECT_EQ(a, a);
  EXPECT_EQ(b, b);
  EXPECT_NE(a, b);

  EXPECT_NE(a, Keyword());

  EXPECT_EQ(a.name(), "a");
  EXPECT_EQ(b.name(), "b");
  EXPECT_EQ(Keyword().name(), "");

  std::unordered_map<Keyword, int> m;
  m[a] = 119;
  m[b] = 120;
  EXPECT_EQ(m[a], 119);
}



