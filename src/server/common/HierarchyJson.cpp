/*
 *  Created on: 2014-04-08
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "HierarchyJson.h"
#include <server/common/Json.h>

namespace sail {
namespace json {

/*namespace {
  CommonJson::Ptr serializeCh(Array<std::shared_ptr<HTree> > ch) {
    Poco::JSON::Array::Ptr arr(new Poco::JSON::Array());
    for (auto c : ch) {
      serialize(c)->addToArray(arr.get());
    }
    return CommonJson::Ptr(new CommonJsonArray(arr));
  }
}*/

CommonJson::Ptr serialize(std::shared_ptr<HTree> &x) {
  Poco::JSON::Object::Ptr obj(new Poco::JSON::Object());
  obj->set("index", x->index());
  obj->set("left", x->left());
  obj->set("right", x->right());
  Array<std::shared_ptr<HTree> > ch = x->children();
  if (ch.hasData()) {
    //serialize(ch)->setObjectField(obj, "children"); // RECURSIVE CALL TO Array-version based on this function.
    serialize(ch)->setObjectField(obj, "children");

    assert(obj->isArray("children"));
    assert(!obj->getArray("children").isNull());
  }
  return CommonJson::Ptr(new CommonJsonObject(obj));
}

CommonJson::Ptr serialize(const HNode &x) {
  Poco::JSON::Object::Ptr obj(new Poco::JSON::Object());
  obj->set("index", x.index());
  obj->set("parent", x.parent());
  obj->set("description", x.description());
  obj->set("code", x.code());
  return CommonJson::Ptr(new CommonJsonObject(obj));
}


void deserialize(CommonJson::Ptr csrc, HNode *dst) {
  Poco::JSON::Object::Ptr src = csrc->toObject()->get();
  int index = src->getValue<int>("index");
  int parent = src->getValue<int>("parent");
  std::string description = src->getValue<std::string>("description");
  std::string code = src->getValue<std::string>("code");
  *dst = HNode(index, parent, code, description);
}

/*Poco::JSON::Array serialize(Array<HNode> src) {
  return serialize(src);
}
void deserialize(Poco::JSON::Array src, Array<HNode> *dst) {
  deserializeArray(src, dst);
}*/

void deserialize(CommonJson::Ptr csrc, std::shared_ptr<HTree> *dst) {
  Poco::JSON::Object::Ptr src = csrc->toObject()->get();
  assert(!src.isNull());
  int index = src->getValue<int>("index");
  int left = src->getValue<int>("left");
  int right = src->getValue<int>("right");
  assert(left < right);
  Array<std::shared_ptr<HTree> > children;
  CommonJson::Ptr ch = CommonJson::getObjectField(src, "children");

  if (bool(ch)) {
    Poco::JSON::Array::Ptr arrptr = ch->toArray()->get();
    if (!arrptr.isNull()) {
      deserialize(ch, &children);
    }
  }

  if (children.empty()) {
    *dst = std::shared_ptr<HTree>(new HLeaves(left, index, right - left));
  } else {
    HInner *hi = new HInner(index, children[0]);
    *dst = std::shared_ptr<HTree>(hi);
    int childCount = children.size();
    for (int i = 1; i < childCount; i++) {
      hi->add(children[i]);
    }
  }
  assert(children.size() == (*dst)->childCount());
}

/*Poco::JSON::Array serialize(Array<std::shared_ptr<HTree> > x) {
  return serializeArray(x);
}

void deserialize(Poco::JSON::Array src, Array<std::shared_ptr<HTree> > *dst) {
  deserializeArray(src, dst);
}*/




}
} /* namespace sail */
