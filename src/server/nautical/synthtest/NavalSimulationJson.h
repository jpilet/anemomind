/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef NAVALSIMULATIONJSON_H_
#define NAVALSIMULATIONJSON_H_

#include <server/common/Json.h>
#include <Poco/Dynamic/Var.h>

namespace sail {
namespace json {

Poco::Dynamic::Var serialize(const BoatSim::FullState &x);
bool deserialize(Poco::Dynamic::Var src, BoatSim::FullState *dst);

template <typename T>
Poco::Dynamic::Var serialize(const CorruptedBoatState::Corruptor<T> &obj) {

}

template <typename T>
bool deserialize(Poco::Dynamic::Var src, CorruptedBoatState::Corruptor<T> *dst) {

}


Poco::Dynamic::Var serialize(const CorruptedBoatState &obj);
bool deserialize(Poco::Dynamic::Var src, CorruptedBoatState *dst);

}
}

#endif /* NAVALSIMULATIONJSON_H_ */
