/*
 *  Created on: 2014-10-11
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "HTreeJson.h"
#include <assert.h>
#include <server/common/Json.h>

#include <server/common/Json.impl.h>

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

  Poco::Dynamic::Var serialize(HTreeWithData h) {
    std::shared_ptr<HTree> x = h.tree();
    Poco::JSON::Object::Ptr obj(new Poco::JSON::Object());
    assert(x->left() < x->right());
    obj->set("code", h.info()[x->index()].code());
    obj->set("description", h.info()[x->index()].description());
    obj->set("left", h.navs()[x->left()].time().toIso8601String());

    // Important: SUBTRACT ONE SO THAT WE INDEX A VALID ELEMENT.
    // In other words, indexing a subrange is like Matlab with both
    // endpoints included in the range.
    obj->set("right", h.navs()[x->right() - 1].time().toIso8601String());


    Array<std::shared_ptr<HTree> > ch = x->children();
    if (ch.hasData()) {
      Poco::Dynamic::Var arr = serializeMapped(ch, h.navs(), h.info());
      obj->set("children", arr);
      assert(obj->isArray("children"));
      assert(!obj->getArray("children").isNull());
    }
    return Poco::Dynamic::Var(obj);
  }
}

Poco::Dynamic::Var serializeMapped(std::shared_ptr<HTree> x, Array<Nav> navs, Array<HNode> nodeInfo) {
  return serialize(HTreeWithData(x, navs, nodeInfo));
}

Poco::Dynamic::Var serializeMapped(Array<std::shared_ptr<HTree> > x, Array<Nav> navs, Array<HNode> nodeInfo) {
  return serialize(x.map<HTreeWithData>([&](std::shared_ptr<HTree> h) {return HTreeWithData(h, navs, nodeInfo);}));
}

}
} /* namespace sail */
