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

/*
 * Note:
 * Some fields will not be serialized, in particular
 * those with std::function objects. We don't know how
 * to serialize/deserialize a generic function object.
 * But that may not be a problem as long as we don't
 * need to use the function object in the deserialized
 * object where it is.
 */
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

Poco::Dynamic::Var serialize(const BoatSimulationSpecs &src);
bool deserialize(Poco::Dynamic::Var src, BoatSimulationSpecs *dst);


}
}

#endif /* NAVALSIMULATIONJSON_H_ */
