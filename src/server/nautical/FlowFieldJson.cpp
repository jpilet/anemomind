/*
 *  Created on: 2014-06-16
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/common/PhysicalQuantityJson.h>
#include <server/nautical/FlowField.h>
#include <server/math/GridJson.h>
#include <server/nautical/FlowFieldJson.h>
#include <server/common/LineKMJson.h>
#include <server/common/Json.h>
#include <server/common/string.h>

namespace sail {
namespace json {

Poco::Dynamic::Var serialize(const FlowField &x) {
  Poco::JSON::Object::Ptr obj(new Poco::JSON::Object());
  Poco::Dynamic::Var gridj = serialize(x.grid());
  Poco::Dynamic::Var flowj = serialize(x.flow());
  obj->set("grid", gridj);
  obj->set("flow", flowj);
  return Poco::Dynamic::Var(obj);
}

bool deserialize(Poco::Dynamic::Var csrc, FlowField *dst) {
  try {
      Poco::JSON::Object::Ptr src = csrc.extract<Poco::JSON::Object::Ptr>();
      Grid3d grid;
      Array<FlowField::FlowVector> flow;
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
