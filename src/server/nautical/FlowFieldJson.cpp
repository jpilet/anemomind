/*
 *  Created on: 2014-06-16
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/common/LineKMJson.h>
#include <server/common/JsonPrimitive.h>
#include <server/common/PhysicalQuantityJson.h>


#include "FlowFieldJson.h"
#include <Poco/JSON/Object.h>
#include <server/nautical/FlowField.h>
#include <server/math/GridJson.h>

namespace sail {
namespace json {

Poco::Dynamic::Var serialize(const FlowField &x) {
  Poco::JSON::Object::Ptr obj(new Poco::JSON::Object());
  obj->set("grid", serialize(x.grid()));
  obj->set("flow", serialize(x.flow()));
  return Poco::Dynamic::Var(x);
}

bool deserialize(Poco::Dynamic::Var csrc, FlowField *dst) {
  try {
      Poco::JSON::Object::Ptr src = csrc.extract<Poco::JSON::Object::Ptr>();
      Grid3d grid;
      Array<FlowField::InternalFlowVector> flow;
      if (!deserialize(src->get("grid"), &grid)) {
        return false;
      }
      if (!deserialize(src->get("flow"), &flow)) {
        return false;
      }
      *dst = FlowField(grid, flow);
      return true;
    } catch (Poco::Exception &e) {
      return false;
    }
}

}
} /* namespace sail */
