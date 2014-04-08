/*
 *  Created on: 2014-04-08
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "HierarchyJson.h"
#include <server/common/Json.h>

namespace sail {
namespace json {

Poco::JSON::Object::Ptr serialize(const HNode &x) {
  Poco::JSON::Object::Ptr obj(new Poco::JSON::Object());
  obj->set("index", x.index());
  obj->set("parent", x.parent());
  obj->set("label", x.label());
  return obj;
}


void deserialize(Poco::JSON::Object::Ptr src, HNode *dst) {
  int index = src->getValue<int>("index");
  int parent = src->getValue<int>("parent");
  std::string label = src->getValue<std::string>("label");
  *dst = HNode(index, parent, label);
}

Poco::JSON::Array serialize(Array<HNode> src) {
  return serializeArray(src);
}
void deserialize(Poco::JSON::Array src, Array<HNode> *dst) {
  deserializeArray(src, dst);
}




}
} /* namespace sail */
