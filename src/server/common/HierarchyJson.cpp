/*
 *  Created on: 2014-04-08
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "HierarchyJson.h"
#include <server/common/Json.h>

namespace sail {
namespace json {


Poco::Dynamic::Var serialize(std::shared_ptr<HTree> &x) {
  Poco::JSON::Object::Ptr obj(new Poco::JSON::Object());
  obj->set("index", x->index());
  obj->set("left", x->left());
  obj->set("right", x->right());
  Array<std::shared_ptr<HTree> > ch = x->children();
  if (ch.hasData()) {
    obj->set("children", serialize(ch));
    assert(obj->isArray("children"));
    assert(!obj->getArray("children").isNull());
  }
  return Poco::Dynamic::Var(obj);
}

Poco::Dynamic::Var serialize(const HNode &x) {
  Poco::JSON::Object::Ptr obj(new Poco::JSON::Object());
  obj->set("index", x.index());
  obj->set("parent", x.parent());
  obj->set("description", x.description());
  obj->set("code", x.code());
  return Poco::Dynamic::Var(obj);
}


bool deserialize(Poco::Dynamic::Var csrc, HNode *dst) {
  try {
    Poco::JSON::Object::Ptr src = csrc.extract<Poco::JSON::Object::Ptr>();
    int index = src->getValue<int>("index");
    int parent = src->getValue<int>("parent");
    std::string description = src->getValue<std::string>("description");
    std::string code = src->getValue<std::string>("code");
    *dst = HNode(index, parent, code, description);
    return true;
  } catch (Poco::Exception &e) {
    return false;
  }
}


bool deserialize(Poco::Dynamic::Var csrc, std::shared_ptr<HTree> *dst) {
  try {
    Poco::JSON::Object::Ptr src = csrc.extract<Poco::JSON::Object::Ptr>();
    assert(!src.isNull());
    int index = src->getValue<int>("index");
    int left = src->getValue<int>("left");
    int right = src->getValue<int>("right");
    if (!(left < right)) {
      return false;
    }
    Array<std::shared_ptr<HTree> > children;
    Poco::Dynamic::Var ch = src->get("children");

    if (ch.isArray()) {
      Poco::JSON::Array::Ptr arrptr = ch.extract<Poco::JSON::Array::Ptr>();
      if (!arrptr.isNull()) {
        if (!deserialize(ch, &children)) {
          return false;
        }
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
    return true;
  } catch (Poco::Exception &e) {
    return false;
  }
}



}
} /* namespace sail */
