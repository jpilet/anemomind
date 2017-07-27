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

struct MyData {
  std::string name;
  int age = 0;
  DYNAMIC_INTERFACE;
};

DYNAMIC_IMPLEMENTATION(
    MyData,
    field("name", name),
    field("age", age));

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

