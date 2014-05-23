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
CommonJson::Ptr serialize(const Grid<N> &grid) {
  CommonJson::Ptr obj = CommonJsonObject::make();
  obj->toObject()->set("inds", serialize(grid.getInds()));
  obj->toObject()->set("ind2Coord", serializeArray(Array<LineKM>(N, grid.ind2Coord())));
  return obj;
}

template <int N>
void deserialize(CommonJson::Ptr cobj, Grid<N> *dst) {
  CommonJsonObject obj = cobj->toObject();
  MDInds<N> inds;
  deserialize(obj->get("inds"), &inds);
  Array<LineKM> ind2Coord(N);
  deserializeArray(obj->get("ind2Coord"), &ind2Coord);
  assert(ind2Coord.size() == N);
  *dst = Grid(inds, ind2Coord.ptr());
}

}
} /* namespace sail */

#endif /* GRIDJSON_H_ */
