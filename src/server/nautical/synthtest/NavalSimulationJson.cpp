/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/synthtest/NavalSimulationJson.h>

namespace sail {
namespace json {

Poco::Dynamic::Var serialize(const BoatSim::FullState &x) {
  Poco::JSON::Object::Ptr obj(new Poco::JSON::Object());
  obj->set("pos", serialize(x.pos));
  obj->set("time", serialize(x.time));
  obj->set("boatOrientation", serialize(x.boatOrientation));
  obj->set("boatAngularVelocity", serialize(x.boatAngularVelocity));
  obj->set("boatSpeedThroughWater", serialize(x.boatSpeedThroughWater));
  obj->set("trueWind", serialize(x.trueWind));
  obj->set("trueCurrent", serialize(x.trueCurrent));
  obj->set("windWrtCurrent", serialize(x.windWrtCurrent));
  obj->set("windAngleWrtWater", serialize(x.windAngleWrtWater));
  obj->set("windSpeedWrtWater", serialize(x.windSpeedWrtWater));
  obj->set("boatMotionThroughWater", serialize(x.boatMotionThroughWater));
  obj->set("boatMotion", serialize(x.boatMotion));
  return obj;
}

bool deserialize(Poco::Dynamic::Var src, BoatSim::FullState *dst) {
  ObjDeserializer deser(src);
  deser.get("pos", &(dst->pos));
  deser.get("time", &(dst->time));
  deser.get("boatOrientation", &(dst->boatOrientation));
  deser.get("boatAngularVelocity", &(dst->boatAngularVelocity));
  deser.get("boatSpeedThroughWater", &(dst->boatSpeedThroughWater));
  deser.get("trueWind", &(dst->trueWind));
  deser.get("trueCurrent", &(dst->trueCurrent));
  deser.get("windWrtCurrent", &(dst->windWrtCurrent));
  deser.get("windAngleWrtWater", &(dst->windAngleWrtWater));
  deser.get("windSpeedWrtWater", &(dst->windSpeedWrtWater));
  deser.get("boatMotionThroughWater", &(dst->boatMotionThroughWater));
  deser.get("boatMotion", &(dst->boatMotion));
  return deser.success();
}


}
}
