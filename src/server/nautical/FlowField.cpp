/*
 *  Created on: 2014-05-22
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "FlowField.h"
#include <server/common/Uniform.h>
#include <server/plot/extra.h>
#include <server/common/string.h>
#include <server/common/ArrayIO.h>

namespace sail {

FlowField::FlowVector FlowField::map(
    Length<double> x, Length<double> y, Duration<double> time) const {
  double localPos[3] = {x.meters(), y.meters(), time.seconds()};
  int inds[Grid3d::WVL];
  double weights[Grid3d::WVL];
  assert(_grid.valid(localPos));
  _grid.makeVertexLinearCombination(localPos, inds, weights);
  InternalFlowVector dst{0, 0};
  for (int i = 0; i < Grid3d::WVL; i++) {
    dst = dst + _flow[inds[i]].scaled(weights[i]);
  }
  return makeFlowVector(dst);
}

FlowField FlowField::generate(Span<Length<double> > xSpan,
                          Span<Length<double> > ySpan,
                          Span<Duration<double> > timeSpan,
                          Length<double> spaceRes,
                          Duration<double> timeRes,
                          Vectorize<Velocity<double>, 2> meanFlow,
                          Velocity<double> maxDif,
                          int spaceSmoothingIters, int timeSmoothingIters) {
  Spand spans[3] = {Spand(xSpan.minv().meters(), xSpan.maxv().meters()),
                    Spand(ySpan.minv().meters(), ySpan.maxv().meters()),
                    Spand(timeSpan.minv().seconds(), timeSpan.maxv().seconds())};
  double res[3] = {spaceRes.meters(), spaceRes.meters(), timeRes.seconds()};
  Grid3d grid(BBox3d(spans), res);

  int n = grid.getVertexCount();

  Array<InternalFlowVector> A(n); {
    double means[2] = {meanFlow[0].metersPerSecond(),
                       meanFlow[1].metersPerSecond()};
    double mdif = maxDif.metersPerSecond();
    Uniform gen[2] = {Uniform(means[0] - mdif, means[0] + mdif),
                      Uniform(means[1] - mdif, means[1] + mdif)};

      for (int i = 0; i < n; i++) {
      A[i] = InternalFlowVector{gen[0].gen(), gen[1].gen()};
    }
  }

  double coefs[3] = {1.0, 1.0, 1.0};
  for (int i = 0; i < spaceSmoothingIters; i++) {
    A = grid.filter3Easy(A, 0, coefs, true);
    A = grid.filter3Easy(A, 1, coefs, true);
  }
  for (int i = 0; i < timeSmoothingIters; i++) {
    A = grid.filter3Easy(A, 2, coefs, true);
  }
  return FlowField(grid, A);
}


MDArray2d FlowField::sampleTimeSliceVectors(Duration<double> t) const {
  const MDInds<3> &inds = _grid.getInds();

  double marg = 1.0e-5;
  LineKM xshrink(0, inds.get(0), marg, inds.get(0) - marg);
  LineKM yshrink(0, inds.get(1), marg, inds.get(1) - marg);

  int count = inds.get(0)*inds.get(1);

  MDArray2d dst(count, 4);
  int counter = 0;
  for (int xi = 0; xi < inds.get(0); xi++) {
    double x = (_grid.getEq(0))(xshrink(xi));
    for (int yi = 0; yi < inds.get(1); yi++) {
      double y = (_grid.getEq(1))(yshrink(yi));
      FlowField::FlowVector vec = map(Length<double>::meters(x),
                                      Length<double>::meters(y),
                                      t);
      dst(counter, 0) = x;
      dst(counter, 1) = y;
      dst(counter, 2) = vec[0].metersPerSecond();
      dst(counter, 3) = vec[1].metersPerSecond();
      counter++;
    }
  }
  assert(counter == count);
  return dst;
}

namespace {
  Spand calcVelocityNormSpan(MDArray2d samples) {
    Spand dst;
    int count = samples.rows();
    for (int i = 0; i < count; i++) {
      double len = sqrt(sqr(samples(i, 2)) + sqr(samples(i, 3)));
      dst.extend(len);
    }
    return dst;
  }

}

void FlowField::plotTimeSlice(Duration<double> time) const {
  MDArray2d samples = sampleTimeSliceVectors(time);

  Spand normSpan = calcVelocityNormSpan(samples);
  std::cout << EXPR_AND_VAL_AS_STRING(normSpan) << std::endl;

  double reflen = 0.9*std::min(_grid.getEq(0).getK(), _grid.getEq(1).getK());
  LineKM velscale(0.0, normSpan.maxv(), 0.0, reflen);


  int count = samples.rows();
  Spand lensp;
  GnuplotExtra plot;
    plot.set_style("dots");
    plot.set_pointsize(12);
    plot.plot(samples.sliceColsTo(2));
    plot.set_style("lines");
    for (int i = 0; i < count; i++) {
      // colmajor
      double xxyy[4] = {samples(i, 0), samples(i, 0) + velscale(samples(i, 2)),
                        samples(i, 1), samples(i, 1) + velscale(samples(i, 3))};
      double len = sqrt(sqr(velscale(samples(i, 2))) + sqr(velscale(samples(i, 3))));
      assert(len > 1.0e-6);
      lensp.extend(len);

      MDArray2d toPlot(2, 2, xxyy);
      plot.plot(toPlot);
    }
  std::cout << EXPR_AND_VAL_AS_STRING(lensp) << std::endl;
  plot.show();
}


} /* namespace sail */
