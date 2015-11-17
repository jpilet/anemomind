/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/math/nonlinear/BandedSolver.h>

namespace sail {
namespace BandedSolver {

MDArray2d initialize(int sampleCount, int dim) {
  MDArray2d x(sampleCount, dim);
  x.setAll(0.0);
  return x;
}


}
}



