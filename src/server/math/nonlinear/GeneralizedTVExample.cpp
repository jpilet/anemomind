/*
 *  Created on: 2014-10-10
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/common/ArgMap.h>
#include <server/common/LineKM.h>
#include <server/math/nonlinear/GeneralizedTV.h>
#include <server/common/Uniform.h>
#include <server/plot/extra.h>

using namespace sail;

namespace {
  constexpr int third = 100;
  constexpr int count = 3*third;

  double testfun(double x) {
    if (x < third) {
      return 0;
    } else if (x < 2*third) {
      return LineKM(third, 2*third, 0, 1)(x);
    }
    return 1;
  }

  Arrayd makeGT() {
    return Arrayd::fill(count, [](int i) {return testfun(i);});
  }

  Arrayd addNoise(Arrayd Y, double noise) {
    Uniform rng(-noise, noise);
    return Y.map<double>([&](double x) {return x + rng.gen();});
  }
}

int main(int argc, const char **argv) {
  ArgMap amap;
  int order = 2;
  double lambda = 1.0;
  double noise = 0.5;
  amap.registerOption("--order", "Set the order of the regularization term")
      .setArgCount(1).store(&order);
  amap.registerOption("--lambda", "Set the regularization weight")
    .setArgCount(1).store(&lambda);
  amap.registerOption("--noise", "Set the amplitude of the noise")
      .setArgCount(1).store(&noise);
  if (amap.parseAndHelp(argc, argv)) {
    GeneralizedTV tv;
    Arrayd Ygt = makeGT();
    Arrayd Ynoisy = addNoise(Ygt, noise);
    Arrayd X = GeneralizedTV::makeDefaultX(count);
    UniformSamples Yfiltered = tv.filter(Ynoisy, order, lambda);

    GnuplotExtra plot;
    plot.plot_xy(X, Ygt, "Ground truth signal");
    plot.plot_xy(X, Ynoisy, "Noisy signal");
    plot.plot_xy(X, Yfiltered.interpolateLinear(X), "Filtered signal");
    plot.show();

    return 0;
  }
  return -1;
}



