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


Poco::Dynamic::Var serialize(const LineKM &x);

template <int N>
Poco::Dynamic::Var serialize(Grid<N> grid) {
  Poco::JSON::Object::Ptr obj(new Poco::JSON::Object());
  obj->set("inds", serialize(grid.getInds()));
  obj->set("ind2Coord", serialize(Array<LineKM>(N, grid.ind2Coord())));
  return Poco::Dynamic::Var(obj);
}

template <int N>
void deserialize(Poco::Dynamic::Var cobj, Grid<N> *dst) {
  Poco::JSON::Object::Ptr obj = cobj.extract<Poco::JSON::Object::Ptr>();
  MDInds<N> inds;
  deserialize(obj->get("inds"), &inds);
  Array<LineKM> ind2Coord(N);
  deserialize(obj->get("ind2Coord"), &ind2Coord);
  assert(ind2Coord.size() == N);
  *dst = Grid<N>(inds, ind2Coord.ptr());
}

}
} /* namespace sail */

#endif /* GRIDJSON_H_ */
