/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/synthtest/NavalSimulationJson.h>
#include <server/common/JsonObjDeserializer.h>


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

template <typename T>
Poco::Dynamic::Var serializeCorr(const CorruptedBoatState::Corruptor<T> &src) {
  Poco::JSON::Object::Ptr obj(new Poco::JSON::Object());
  obj->set("stddev", serialize(src.distrib().stddev()));
  obj->set("scale", serialize(src.scale()));
  obj->set("offset", serialize(src.offset()));
  return obj;
}

template <typename T>
bool deserializeCorr(Poco::Dynamic::Var src, CorruptedBoatState::Corruptor<T> *dst) {
  ObjDeserializer deser(src);
  decltype(dst->distrib().mean()) mean;
  decltype(dst->distrib().mean()) stddev;
  deser.get("stddev", &stddev);
  decltype(dst->scale()) scale;
  decltype(dst->offset()) offset;
  deser.get("scale", &scale);
  deser.get("offset", &offset);
  *dst = CorruptedBoatState::Corruptor<T>(scale, offset, stddev);
  return deser.success();
}

Poco::Dynamic::Var serialize(const CorruptedBoatState::Corruptor<Angle<double> > &src) {
  return serializeCorr(src);
}

Poco::Dynamic::Var serialize(const CorruptedBoatState::Corruptor<Velocity<double> > &src) {
  return serializeCorr(src);
}

bool deserialize(Poco::Dynamic::Var src, CorruptedBoatState::Corruptor<Angle<double> > *dst) {
  return deserializeCorr(src, dst);
}

bool deserialize(Poco::Dynamic::Var src, CorruptedBoatState::Corruptor<Velocity<double> > *dst) {
  return deserializeCorr(src, dst);
}



Poco::Dynamic::Var serialize(const CorruptedBoatState &src) {
  Poco::JSON::Object::Ptr obj(new Poco::JSON::Object());
  obj->set("trueState", serialize(src.trueState()));
  obj->set("corruptedNav", serialize(src.nav()));
  return obj;
}

bool deserialize(Poco::Dynamic::Var src, CorruptedBoatState *dst) {
  ObjDeserializer deser(src);
  BoatSim::FullState trueState;
  Nav nav;
  deser.get("trueState", &trueState);
  deser.get("corruptedNav", &nav);
  if (deser.success()) {
    *dst = CorruptedBoatState(trueState, nav);
    return true;
  }
  return false;
}

BoatSim::FullState _trueState;
  Nav _corruptedNav;

}
}
