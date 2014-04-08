/*
 *  Created on: 2014-04-08
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */


#include <server/common/HierarchyJson.h>
#include <gtest/gtest.h>

using namespace sail;

TEST(HierarchyJsonTest, TestSimple) {
  HNode x(1, 2, "testnode");
  Poco::JSON::Object::Ptr obj = json::serialize(x);
  HNode y;
  json::deserialize(obj, &y);
  EXPECT_EQ(x.index(), y.index());
  EXPECT_EQ(x.parent(), y.parent());
  EXPECT_EQ(x.label(), y.label());
}

TEST(HierarchyJsonTest, TestSimpleArray) {
  HNode x(1, 2, "testnode");
  Array<HNode> X = Array<HNode>::args(x);
  Poco::JSON::Array obj = json::serialize(X);
  Array<HNode> Y;
  json::deserialize(obj, &Y);

  HNode y = Y.first();

  EXPECT_EQ(x.index(), y.index());
  EXPECT_EQ(x.parent(), y.parent());
  EXPECT_EQ(x.label(), y.label());
}


