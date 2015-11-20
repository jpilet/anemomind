/*
 *  Created on: 2014-10-10
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/common/ArgMap.h>
#include <server/common/LineKM.h>
#include <server/math/nonlinear/GeneralizedTVAuto.h>
#include <server/plot/extra.h>
#include <server/common/ScopedLog.h>
#include <server/common/string.h>
#include <server/common/Functional.h>

using namespace sail;

namespace {
  /*
   * Some definitions related to the shape of the test
   * signal. It consists of 5 parts, where the two
   * first parts are 0, the third part is a slope from
   * 0 to 1 and the last two parts are 1.
   */
  constexpr int partCount = 100;
  constexpr int sampleCount = 5*partCount;

  double testfun(double x) {
    int a = 2*partCount;
    int b = a + partCount;
    if (x < a) {
      return 0;
    } else if (x < b) {
      return LineKM(a, b, 0, 1)(x);
    }
    return 1;
  }

  Arrayd makeGT() {
    return Arrayd::fill(sampleCount, [](int i) {return testfun(i);});
  }

  Arrayd addNoise(Arrayd Y, double noise, std::default_random_engine &engine) {
    std::uniform_real_distribution<double> distrib(-noise, noise);
    return toArray(sail::map([&](double x) {return x + distrib(engine);}, Y));
  }
}

int main(int argc, const char **argv) {
  std::default_random_engine engine(0);
  ArgMap amap;
  int order = 2;
  double lambda = 60.0;
  double noise = 0.5;
  int iters = 30;
  int verbosity = 0;
  amap.registerOption("--order", "Set the order of the regularization term")
      .setArgCount(1).store(&order);
  amap.registerOption("--lambda", "Set the regularization weight")
    .setArgCount(1).store(&lambda);
  amap.registerOption("--noise", "Set the amplitude of the noise")
      .setArgCount(1).store(&noise);
  amap.registerOption("--iters", "Set the maximum number of iterations")
      .setArgCount(1).store(&iters);
  amap.registerOption("--auto", "Automatic tuning of regularization");
  amap.registerOption("--verbosity", "Set verbosity")
      .setArgCount(1).store(&verbosity);

  amap.setHelpInfo("This program illustrates generalized TV filtering\n"
                   "of a 1-D signal. The order option sets the order\n"
                   "of the differential operator. Standard TV denoising\n"
                   "has an order of 1. \n"
                   "\n"
                   "The default options should result in a filtered signal\n"
                   "of a signal that faithfully approximates the original\n"
                   "\n"
                   "An order of 1, however, can result in poor\n"
                   "recovery if the underlying signal is no piecewise\n"
                   "constant. Try for instance to call this program as:\n"
                   "\n"
                   "./math_nonlinear_GeneralizedTVExample --order 1 --lambda 4\n"
                   "\n"
                   "The recovered signal will have a staircase like shape.\n");
  if (amap.parse(argc, argv) != ArgMap::Error) {
    ScopedLog::setDepthLimit(verbosity);
    GeneralizedTV tv(iters);
    Arrayd Ygt = makeGT();
    Arrayd Ynoisy = addNoise(Ygt, noise, engine);
    Arrayd X = GeneralizedTV::makeDefaultX(sampleCount);

    UniformSamplesd Yfiltered;
    if (amap.optionProvided("--auto")) {
      GeneralizedTVAuto autotv(engine, tv);
      Yfiltered = autotv.filter(Ynoisy, order);
    } else {
      Yfiltered = tv.filter(Ynoisy, order, lambda);
    }


    GnuplotExtra plot;
    plot.set_style("lines");
    plot.plot_xy(X, Ygt, "Ground truth signal");
    plot.plot_xy(X, Ynoisy, "Noisy signal");
    plot.plot_xy(X, Yfiltered.interpolateLinear(X), "Filtered signal");
    plot.show();

    return 0;
  }
  return -1;
}



