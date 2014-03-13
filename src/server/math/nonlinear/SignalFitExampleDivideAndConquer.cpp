/*
 *  Created on: 2014-03-13
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */
#include <server/math/nonlinear/GridFitterTestData.h>
#include <server/math/nonlinear/SignalFit.h>
#include <server/plot/extra.h>
#include <server/common/string.h>
#include <iostream>

namespace sail {

namespace {
  MDArray2d makeXY(LineStrip strip, Arrayd vertices) {
    int n = vertices.size();
    assert(n == strip.getVertexCount());
    MDArray2d XY(n, 2);
    for (int i = 0; i < n; i++) {
      XY(i, 0) = i;
      XY(i, 1) = vertices[i];
    }
    return XY;
  }
}

void sfexDAC() {
  GridFitterTestData data;

  double marg = 1.0;
  LineStrip strip(Span(data.minX() - marg, data.maxX() + marg), 0.1);

  double reg = 1.0;
  double reg1 = reg;
  double reg2 = reg;

  reg2 = 0;

  Arrayd vertices = fitLineStrip(strip, reg1, reg2, data.X, data.Ynoisy);

  std::cout << EXPR_AND_VAL_AS_STRING(vertices.size()) << std::endl;

  MDArray2d XY = makeXY(strip, vertices);

  GnuplotExtra plot;
    plot.set_style("lines");
    plot.plot(XY);
  plot.show();
}



}

int main() {
  sail::sfexDAC();
  return 0;
}


