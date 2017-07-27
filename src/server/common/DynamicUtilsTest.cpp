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

  /*Poco::Dynamic::Var toDynamic() {
    return Poco::Dynamic::Var(Poco::JSON::Object::Ptr(new Poco::JSON::Object()));
  }*/

  DYNAMIC_INTERFACE;
};

DYNAMIC_IMPLEMENTATION(
    MyData,
    field("name", name),
    field("age", age));

Poco::Dynamic::Var makeDynamic() {
  return Poco::Dynamic::Var(Poco::JSON::Object::Ptr(new Poco::JSON::Object()));
}




TEST(DynamicTest, TestStructSerialization) {
  MyData x;
  x.name = "Signe";
  x.age = 13;

  std::cout << "Convert it to dynamic" << std::endl;
  auto d = x.toDynamic();
  //auto d = makeDynamic();
  std::cout << "Done" << std::endl;

  std::ofstream file("/tmp/mydata.json");
  //outputJson(x, &file);

  Poco::Dynamic::Var y(Poco::JSON::Object::Ptr(new Poco::JSON::Object()));
  auto z = Poco::Dynamic::Var(Poco::JSON::Object::Ptr(new Poco::JSON::Object()));
  auto w = makeDynamic();
  //outputJson(makeDynamic(), &file); // works
  //outputJson(y, &file); // works
  //outputJson(z, &file); // works
  //outputJson(w, &file);
  outputJson(d, &file);


}

