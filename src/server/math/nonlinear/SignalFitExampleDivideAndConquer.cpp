/*
 *  Created on: 2014-03-13
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */
#include <server/math/nonlinear/GridFitterTestData.h>
#include <server/math/nonlinear/SignalFit.h>

namespace sail {



void sfexDAC() {
  GridFitterTestData data;

  double marg = 1.0;
  LineStrip strip(Span(data.minX() - marg, data.maxX() + marg), 100);

  double reg1 = 1.0;
  double reg2 = 1.0;

  Arrayd vertices = fitLineStrip(strip, reg1, reg2, data.X, data.Ynoisy);
}



}

int main() {
  sail::sfexDAC();
  return 0;
}


