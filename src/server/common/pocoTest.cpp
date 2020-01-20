/*
 *  Created on: 2014-03-26
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <gtest/gtest.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/ParseHandler.h>
#include <string>

using namespace Poco::JSON;

//http://stackoverflow.com/questions/15387154/correct-usage-of-poco-c-json-for-parsing-data
//Based on this answer: http://stackoverflow.com/a/15420512
TEST(PocoTest, PocoJsonStackoverflow) {
  // array of objects
  std::string jsondata = "[ {\"test\" : 0}, { \"test1\" : [1, 2, 3], \"test2\" : 4 } ]";
  Parser parser;
  Poco::SharedPtr<ParseHandler> handler(new ParseHandler());

  parser.setHandler(handler);

  parser.parse(jsondata);
  Poco::Dynamic::Var result = handler->asVar();
  Array::Ptr arr = result.extract<Array::Ptr>();
  Object::Ptr object = arr->getObject(0);//
  EXPECT_EQ(object->getValue<int>("test"), 0);
  object = arr->getObject(1);
  arr = object->getArray("test1");
  result = arr->get(0);
  EXPECT_TRUE(result.isInteger());
  EXPECT_EQ(result.extract<int64_t>(), 1);
}



