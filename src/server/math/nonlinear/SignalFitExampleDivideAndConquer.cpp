/*
 *  Created on: 2014-03-13
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */
#include <server/math/nonlinear/GridFitterTestData.h>
#include <server/math/nonlinear/SignalFit.h>
#include <server/plot/extra.h>
#include <server/common/string.h>
#include <server/math/nonlinear/LevmarSettings.h>
#include <iostream>
#include <server/common/ArrayIO.h>

namespace sail {

namespace {
  MDArray2d makeXY(LineStrip strip, Arrayd vertices) {
    int n = vertices.size();
    assert(n == strip.getVertexCount());
    MDArray2d coords = strip.getGridVertexCoords();
    MDArray2d XY(n, 2);
    for (int i = 0; i < n; i++) {
      XY(i, 0) = coords(i, 0);
      XY(i, 1) = vertices[i];
    }
    return XY;
  }

  MDArray2d makeXY(Arrayd X, Arrayd Y) {
    int count = X.size();
    assert(count == Y.size());
    MDArray2d XY(count, 2);
    for (int i = 0; i < count; i++) {
      XY(i, 0) = X[i];
      XY(i, 1) = Y[i];
    }
    return XY;
  }
}

void sfexDAC() {
  GridFitterTestData data;

  double marg = 0.1;
  LineStrip strip(Span(data.minX() - marg, data.maxX() + marg), 0.01);

  Arrayd regs;
  Arrayi orders = makeDefaultRegOrders(2);

  //Arrayd vertices = fitLineStrip(strip, regs, data.X, data.Ynoisy);
  LevmarSettings settings;
  SignalFitResults res = fitLineStripAutoTune(strip, orders, regs,
      data.X, data.Ynoisy, data.splits, settings);

  std::cout << EXPR_AND_VAL_AS_STRING(res.regWeights) << std::endl;

  std::cout << EXPR_AND_VAL_AS_STRING(res.vertices.size()) << std::endl;

  MDArray2d XY = makeXY(strip, res.vertices);

  GnuplotExtra plot;
    plot.set_style("lines");
    plot.plot(XY);
    plot.set_style("points");
    plot.plot(makeXY(data.X, data.Ynoisy));
  plot.show();
}



}

int main() {
  sail::sfexDAC();
  return 0;
}


