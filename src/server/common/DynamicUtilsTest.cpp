/*
 * DynamicUtilsTest.cpp
 *
 *  Created on: 15 Jun 2017
 *      Author: jonas
 */

#include <gtest/gtest.h>
#include <server/common/DynamicUtils.h>
#include <fstream>

using namespace sail;

typedef std::enable_if<true, int>::type x;

static_assert(IsSequenceLike<std::vector<int>>::value, "");
static_assert(!IsSequenceLike<int>::value, "");
static_assert(!IsSequenceLike<std::string>::value, "");
static_assert(!IsSequenceLike<std::map<int, int>>::value, "");

struct MyData {
  std::string name;
  int age = 0;
  std::vector<int> values;
  DYNAMIC_INTERFACE;
};

DYNAMIC_IMPLEMENTATION(
    MyData,
    field("name", name),
    field("age", age)/*,
    field("values", values)*/);

TEST(DynamicTest, Sequence) {
  std::vector<int> mjao{9, 20, 11};
  Poco::Dynamic::Var d;
  EXPECT_TRUE(EnabledSequenceToDynamic<
      std::vector<int>>::apply(mjao, &d));
  std::vector<int> mjao2;
  EXPECT_TRUE(EnabledSequenceFromDynamic<
    std::vector<int>>::apply(d, &mjao2));

  EXPECT_EQ(mjao.size(), mjao2.size());
  for (int i = 0; i < 3; i++) {
    EXPECT_EQ(mjao[i], mjao2[i]);
  }
}

TEST(DynamicTest, TestStructSerialization) {
  {
    MyData x;
    x.name = "Signe";
    x.age = 13;

    std::ofstream file("/tmp/mydata.json");
    EXPECT_TRUE(writeJson(x, &file));
  }{
    MyData x;
    std::ifstream file("/tmp/mydata.json");
    EXPECT_TRUE(readJson(&file, &x));
    EXPECT_EQ(x.name, "Signe");
    EXPECT_EQ(x.age, 13);
  }
}

