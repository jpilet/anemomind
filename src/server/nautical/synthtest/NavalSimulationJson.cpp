/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/synthtest/NavalSimulationJson.h>
#include <server/nautical/NavJson.h>
#include <server/common/PhysicalQuantityJson.h>
#include <server/nautical/GeographicReferenceJson.h>
#include <server/common/TimeStampJson.h>
#include <server/common/JsonObjDeserializer.h>
#include <server/common/Json.impl.h>


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

Poco::Dynamic::Var serialize(const BoatCharacteristics &src) {
  Poco::JSON::Object::Ptr obj(new Poco::JSON::Object());
  obj->set("keelRudderDistance", serialize(src.keelRudderDistance));
  obj->set("rudderResistanceCoef", serialize(src.rudderResistanceCoef));
  obj->set("halfTargetSpeedTime", serialize(src.halfTargetSpeedTime));
  obj->set("rudderCorrectionCoef", serialize(src.rudderCorrectionCoef));
  obj->set("rudderFineTune", serialize(src.rudderFineTune));
  obj->set("boatReactiveness", serialize(src.boatReactiveness));
  obj->set("rudderMaxAngle", serialize(src.rudderMaxAngle));
  obj->set("correctionThreshold", serialize(src.correctionThreshold));
  return obj;
}

bool deserialize(Poco::Dynamic::Var src, BoatCharacteristics *dst) {
  ObjDeserializer deser(src);
  deser.get("keelRudderDistance", &(dst->keelRudderDistance));
  deser.get("rudderResistanceCoef", &(dst->rudderResistanceCoef));
  deser.get("halfTargetSpeedTime", &(dst->halfTargetSpeedTime));
  deser.get("rudderCorrectionCoef", &(dst->rudderCorrectionCoef));
  deser.get("rudderFineTune", &(dst->rudderFineTune));
  deser.get("boatReactiveness", &(dst->boatReactiveness));
  deser.get("rudderMaxAngle", &(dst->rudderMaxAngle));
  deser.get("correctionThreshold", &(dst->correctionThreshold));

  // Since we don't know how to serialize/deserialize
  // a generic std::function object, make sure
  // that it is empty.
  dst->targetSpeedFun = std::function<Velocity<double>(Angle<double>,Velocity<double>)>();

  return deser.success();
}

Poco::Dynamic::Var serialize(const BoatSimulationSpecs::TwaDirective &src) {
  Poco::JSON::Object::Ptr obj(new Poco::JSON::Object());
  obj->set("duration", serialize(src.duration));
  obj->set("srcTwa", serialize(src.srcTwa));
  obj->set("dstTwa", serialize(src.dstTwa));
  return obj;
}

bool deserialize(Poco::Dynamic::Var src, BoatSimulationSpecs::TwaDirective *obj) {
  ObjDeserializer deser(src);
  deser.get("duration", &(obj->duration));
  deser.get("srcTwa", &(obj->srcTwa));
  deser.get("dstTwa", &(obj->dstTwa));
  return deser.success();
}

Poco::Dynamic::Var serialize(const CorruptedBoatState::CorruptorSet &src) {
  Poco::JSON::Object::Ptr obj(new Poco::JSON::Object());
  obj->set("awa", serialize(src.awa));
  obj->set("magHdg", serialize(src.magHdg));
  obj->set("gpsBearing", serialize(src.gpsBearing));
  obj->set("aws", serialize(src.aws));
  obj->set("watSpeed", serialize(src.watSpeed));
  obj->set("gpsSpeed", serialize(src.gpsSpeed));
  return obj;
}

bool deserialize(Poco::Dynamic::Var src, CorruptedBoatState::CorruptorSet *dst) {
  ObjDeserializer deser(src);
  deser.get("awa", &(dst->awa));
  deser.get("magHdg", &(dst->magHdg));
  deser.get("gpsBearing", &(dst->gpsBearing));
  deser.get("aws", &(dst->aws));
  deser.get("watSpeed", &(dst->watSpeed));
  deser.get("gpsSpeed", &(dst->gpsSpeed));
  return deser.success();
}

Poco::Dynamic::Var serialize(const BoatSimulationSpecs &src) {
  Poco::JSON::Object::Ptr obj(new Poco::JSON::Object());
  obj->set("characteristics", serialize(src.characteristics()));
  obj->set("dirs", serialize(src.dirs()));
  obj->set("corruptors", serialize(src.corruptors()));
  obj->set("boatId", serialize(src.boatId()));
  obj->set("samplingPeriod", serialize(src.samplingPeriod()));
  obj->set("stepsPerSample", serialize(src.stepsPerSample()));
  return obj;
}

bool deserialize(Poco::Dynamic::Var src, BoatSimulationSpecs *dst) {
  BoatCharacteristics ch;
  Array<BoatSimulationSpecs::TwaDirective> dirs;
  CorruptedBoatState::CorruptorSet corruptors;
  Nav::Id boatId;
  Duration<double> samplingPeriod;
  int stepsPerSample;

  ObjDeserializer deser(src);
  deser.get("characteristics", &ch);
  deser.get("dirs", &dirs);
  deser.get("corruptors", &corruptors);
  deser.get("boatId", &boatId);
  deser.get("samplingPeriod", &samplingPeriod);
  deser.get("stepsPerSample", &stepsPerSample);
  if (deser.success()) {
    *dst = BoatSimulationSpecs(ch, dirs, corruptors, boatId, samplingPeriod, stepsPerSample);
    return true;
  }
  return false;
}

Poco::Dynamic::Var serialize(const NavalSimulation::BoatData &src) {
  Poco::JSON::Object::Ptr obj(new Poco::JSON::Object());
  obj->set("specs", src.specs());
  obj->set("states", src.states());
  return obj;
}

bool deserialize(Poco::Dynamic::Var src, NavalSimulation::BoatData *dst) {
  BoatSimulationSpecs specs;
  Array<CorruptedBoatState> states;

  ObjDeserializer deser(src);
  deser.get("specs", &specs);
  deser.get("states", &states);
  if (deser.success()) {
    *dst = NavalSimulation::BoatData(specs, states);
    return true;
  }
  return false;
}

Poco::Dynamic::Var serialize(const NavalSimulation &src) {
  Poco::JSON::Object::Ptr obj(new Poco::JSON::Object());
  obj->set("desc", src.description());
  obj->set("geoRef", src.geoRef());
  obj->set("simulationStartTime", src.simulationStartTime());
  obj->set("boatData", src.boatData());
  return obj;
}

bool deserialize(Poco::Dynamic::Var src, NavalSimulation *dst) {
  std::string desc;
  GeographicReference geoRef;
  TimeStamp simulationStartTime;
  Array<NavalSimulation::BoatData> boatData;

  ObjDeserializer deser(src);
  deser.get("desc", &desc);
  deser.get("geoRef", &geoRef);
  deser.get("simulationStartTime", &simulationStartTime);
  deser.get("boatData", &boatData);
  if (deser.success()) {
    *dst = NavalSimulation(desc, geoRef, simulationStartTime, boatData);
    return true;
  }
  return false;
}


}
}
