/*
 *  Created on: 2014-05-22
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef GRIDJSON_H_
#define GRIDJSON_H_

#include <server/common/LineKMJson.h>
#include <server/common/MDIndsJson.h>


namespace sail {
namespace json {


template <int N>
Poco::Dynamic::Var serialize(Grid<N> grid) {
  Poco::JSON::Object::Ptr obj(new Poco::JSON::Object());
  obj->set("inds", serialize(grid.getInds()));
  obj->set("ind2Coord", serialize(Array<LineKM>(N, grid.ind2Coord())));
  return Poco::Dynamic::Var(obj);
}

template <int N>
bool deserialize(Poco::Dynamic::Var cobj, Grid<N> *dst) {
  try {
    Poco::JSON::Object::Ptr obj = cobj.extract<Poco::JSON::Object::Ptr>();
    MDInds<N> inds;
    if (!deserialize(obj->get("inds"), &inds)) {
      return false;
    }
    Array<LineKM> ind2Coord(N);
    if (!deserialize(obj->get("ind2Coord"), &ind2Coord)) {
      return false;
    }
    if (!(ind2Coord.size() == N)) {
      return true;
    }
    *dst = Grid<N>(inds, ind2Coord.ptr());
    return true;
  } catch (Poco::Exception &e) {
    return false;
  }
}

}
} /* namespace sail */

#endif /* GRIDJSON_H_ */
