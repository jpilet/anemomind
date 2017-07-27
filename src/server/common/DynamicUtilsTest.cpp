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

  double k = 0.0;

  std::map<std::string, int> mappedValues;

  std::shared_ptr<int> ptrA, ptrB;

  DYNAMIC_INTERFACE;
};

DYNAMIC_IMPLEMENTATION(
    MyData,
    field("name", name),
    field("k", k),
    field("age", age),
    field("values", values),
    field("mapped_values", mappedValues),
    field("ptr_a", ptrA),
    field("ptr_b", ptrB));

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

TEST(DynamicTest, MapTest) {
  std::map<std::string, int> m{
    {"a", 0},
    {"b", 1}
  };

  Poco::Dynamic::Var d;
  EnabledMapToDynamic<std::map<std::string, int>>::apply(m, &d);

  std::map<std::string, int> m2;
  EnabledMapFromDynamic<std::map<std::string, int>>::apply(d, &m2);

  EXPECT_EQ(m2.size(), 2);
  EXPECT_EQ(m2["a"], 0);
  EXPECT_EQ(m2["b"], 1);
}

TEST(DynamicTest, TestStructSerialization) {
  {
    MyData x;
    x.name = "Signe";
    x.age = 13;
    x.values = std::vector<int>{9, 12};
    x.k = 1111.3;
    x.mappedValues = {{"abra", 119}};
    x.ptrB = std::make_shared<int>(222);

    std::ofstream file("/tmp/mydata.json");
    EXPECT_TRUE(writeJson(x, &file));
  }{
    MyData x;
    std::ifstream file("/tmp/mydata.json");
    EXPECT_TRUE(readJson(&file, &x));
    EXPECT_EQ(x.name, "Signe");
    EXPECT_EQ(x.age, 13);
    EXPECT_EQ(x.values.size(), 2);
    EXPECT_EQ(x.values[0], 9);
    EXPECT_EQ(x.values[1], 12);
    EXPECT_NEAR(x.k, 1111.3, 1.0e-5);
    EXPECT_EQ(x.mappedValues["abra"], 119);
    EXPECT_FALSE(x.ptrA);
    EXPECT_EQ(*(x.ptrB), 222);
  }
}

