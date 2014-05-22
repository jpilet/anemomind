/*
 *  Created on: 2014-05-22
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef GRIDJSON_H_
#define GRIDJSON_H_

#include <server/math/Grid.h>
#include <server/common/MDIndsJson.h>

namespace sail {
namespace json {

template <int N>
Poco::JSON::Object::Ptr serialize(const Grid<N> &grid) {
  Poco::JSON::Object::Ptr obj(new Poco::JSON::Object());
  obj->set("inds", serialize(grid.getInds()));
  obj->set("ind2Coord", serializeArray(Array<LineKM>(N, grid.ind2Coord())));
  return obj;
}

template <int N>
void deserialize(Poco::JSON::Object::Ptr obj, Grid<N> *dst) {
  MDInds<N> inds;
  deserialize(obj->getObject("inds"), &inds);
  Array<LineKM> ind2Coord(N);
  deserializeArray(obj->getArray("ind2Coord"), &ind2Coord);
  assert(ind2Coord.size() == N);
  *dst = Grid(inds, ind2Coord.ptr());
}

}
} /* namespace sail */

#endif /* GRIDJSON_H_ */
