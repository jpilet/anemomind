/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef NAVALSIMULATIONJSON_H_
#define NAVALSIMULATIONJSON_H_

#include <server/nautical/synthtest/NavalSimulation.h>
#include <server/common/Json.h>
#include <Poco/Dynamic/Var.h>

namespace sail {
namespace json {

Poco::Dynamic::Var serialize(const BoatSim::FullState &x);
bool deserialize(Poco::Dynamic::Var src, BoatSim::FullState *dst);

Poco::Dynamic::Var serialize(const CorruptedBoatState::Corruptor<Angle<double> > &src);
Poco::Dynamic::Var serialize(const CorruptedBoatState::Corruptor<Velocity<double> > &src);

bool deserialize(Poco::Dynamic::Var src,
    CorruptedBoatState::Corruptor<Angle<double> > *dst);
bool deserialize(Poco::Dynamic::Var src,
    CorruptedBoatState::Corruptor<Velocity<double> > *dst);

Poco::Dynamic::Var serialize(const CorruptedBoatState &obj);
bool deserialize(Poco::Dynamic::Var src, CorruptedBoatState *dst);

Poco::Dynamic::Var serialize(const BoatSimulationSpecs::TwaDirective &obj);
bool deserialize(Poco::Dynamic::Var src, BoatSimulationSpecs::TwaDirective *dst);

Poco::Dynamic::Var serialize(const CorruptedBoatState::CorruptorSet &obj);
bool deserialize(Poco::Dynamic::Var src, const CorruptedBoatState::CorruptorSet *dst);


}
}

#endif /* NAVALSIMULATIONJSON_H_ */
