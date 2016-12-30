/*
 * GameCalib.h
 *
 *  Created on: 30 Dec 2016
 *      Author: jonas
 */

#ifndef SERVER_NAUTICAL_CALIB_GAMECALIB_H_
#define SERVER_NAUTICAL_CALIB_GAMECALIB_H_

#include <server/nautical/calib/CalibDataChunk.h>
#include <server/math/nonlinear/GameSolver.h>
#include <server/common/DOMUtils.h>

namespace sail {
namespace GameCalib {

enum class PlayerType {
  Wind,
  Current,
  IMU,
  Leeway
};

struct Settings {
  Duration<double> currentSamplingPeriod = 1.0_minutes;
  Duration<double> windSamplingPeriod = 1.0_minutes;

  GameSolver::Settings solverSettings;
  std::set<PlayerType> playerTypesToConsider = {PlayerType::Current};
};

void optimize(
    const Array<CalibDataChunk> &chunks,
    const Settings &settings,
    DOM::Node *dst);

}
}

#endif /* SERVER_NAUTICAL_CALIB_GAMECALIB_H_ */
