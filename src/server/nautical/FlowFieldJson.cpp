/*
 *  Created on: May 23, 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "FlowFieldJson.h"
#include <server/math/GridJson.h>
#include <server/common/PhysicalQuantityJson.h>

namespace sail {
namespace json {


CommonJson::Ptr serialize(const Vectorize<double, 2> &x) {
  return serialize(Arrayd(2, x.data()));
}

CommonJson::Ptr serialize(const FlowField &x) {
  CommonJson::Ptr obj = CommonJsonObject::make();
  obj->toObject()->set("grid", serialize(x.grid()));
  obj->toObject()->set("flow", serialize(x.flow()));
  return obj;
}

void deserialize(CommonJson::Ptr obj, FlowField *dst) {
  Array<FlowField::InternalFlowVector> flow;
  deserialize(obj->toObject()->get("flow"), &flow);

  Grid3d grid;
  deserialize(obj->toObject()->get("grid"), &grid);

  *dst = FlowField(grid, flow);
}


}
} /* namespace sail */
