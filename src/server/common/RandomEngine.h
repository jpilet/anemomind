/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef RANDOMENGINE_H_
#define RANDOMENGINE_H_

#include <random>

namespace {

class RandomEngine {
 public:
  typedef std::default_random_engine EngineType;

  static EngineType globalEngine;

  /*
   * Example use case:
   *
   *  double randomNumberBetween0And1(RandomEngine::EngineType *optional = nullptr) {
   *    RandomEngine::EngineType &engine = RandomEngine::get(optional);
   *    std::uniform_real_distribution<double> distrib(0, 1);
   *    return distrib(engine);
   *  }
   *
   */
  static EngineType &get(EngineType *optional) {
    if (optional == nullptr) {
      return globalEngine;
    }
    return *optional;
  }
};

} /* namespace mmm */

#endif /* RANDOMENGINE_H_ */
