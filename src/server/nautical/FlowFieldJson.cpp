/*
 *  Created on: May 23, 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "FlowFieldJson.h"
#include <server/math/GridJson.h>
#include <server/common/PhysicalQuantityJson.h>

namespace sail {
namespace json {


CommonJson::Ptr serializeVd2(Vectorize<double, 2> x) {
  return serialize(Arrayd(2, x.data()));
}

CommonJson::Ptr serialize(const FlowField &x) {
  CommonJson::Ptr obj = CommonJsonObject::make();
  obj->toObject()->set("grid", serialize(x.grid()));

  // Hack to make it compile. I don't no why it won't compile when just calling serialize(x.flow())
  std::function<CommonJson::Ptr(Vectorize<double, 2>)> custom = serializeVd2;
  obj->toObject()->set("flow", serialize(x.flow(), custom));

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
