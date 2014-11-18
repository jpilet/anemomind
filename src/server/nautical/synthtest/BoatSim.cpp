/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "BoatSim.h"
#include <cassert>

namespace sail {


void BoatSimulator::eval(double *Xin, double *Fout, double *Jout) {
  assert(Jout == nullptr);
  BoatSimulationState &state = *((BoatSimulationState *)Xin);
  BoatSimulationState &deriv = *((BoatSimulationState *)Fout);
}

}
