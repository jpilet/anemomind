/*
 *  Created on: 2014-10-11
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "HTreeJson.h"
#include <assert.h>
#include <server/common/Json.h>

namespace sail {
namespace json {

namespace {
  class HTreeWithData {
   public:
    HTreeWithData() {}
    HTreeWithData(std::shared_ptr<HTree> t, Array<Nav> n, Array<HNode> nodeInfo) :
      _tree(t), _navs(n), _info(nodeInfo) {}
    const Array<Nav> &navs() const {return _navs;}
    std::shared_ptr<HTree> tree() const {return _tree;}
    Array<HNode> info() const {return _info;}
   private:
    std::shared_ptr<HTree> _tree;
    Array<Nav> _navs;
    Array<HNode> _info;
  };

  Poco::JSON::Object::Ptr serialize(HTreeWithData h) {
    std::shared_ptr<HTree> x = h.tree();
    Poco::JSON::Object::Ptr obj(new Poco::JSON::Object());
    obj->set("index", h.info()[x->index()].code());
    obj->set("left", h.navs()[x->left()].id());
    obj->set("right", h.navs()[x->right()].id());
    Array<std::shared_ptr<HTree> > ch = x->children();
    if (ch.hasData()) {
      Poco::JSON::Array::Ptr arr(new Poco::JSON::Array(serializeMapped(ch, h.navs(), h.info())));
      obj->set("children", arr);
      assert(obj->isArray("children"));
      assert(!obj->getArray("children").isNull());
    }
    return obj;
  }
}

Poco::JSON::Object::Ptr serializeMapped(std::shared_ptr<HTree> x, Array<Nav> navs, Array<HNode> nodeInfo) {
  return serialize(HTreeWithData(x, navs, nodeInfo));
}

Poco::JSON::Array serializeMapped(Array<std::shared_ptr<HTree> > x, Array<Nav> navs, Array<HNode> nodeInfo) {
  return serializeArray(x.map<HTreeWithData>([&](std::shared_ptr<HTree> h) {return HTreeWithData(h, navs, nodeInfo);}));
}

}
} /* namespace sail */
