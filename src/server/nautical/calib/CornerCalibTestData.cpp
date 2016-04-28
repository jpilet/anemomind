/*
 * CornerCalibTestData.cpp
 *
 *  Created on: Apr 21, 2016
 *      Author: jonas
 */

#include <server/nautical/calib/CornerCalibTestData.h>

namespace sail {
namespace CornerCalibTestData {

  Arrayd rng{-0.736924, -0.0826997, -0.562082, 0.357729, 0.869386,
    0.0388327, -0.930856, 0.0594004, -0.984604, -0.866316, 0.373545,
    0.860873, 0.0538576, 0.307838, 0.402381, 0.524396, -0.905071,
    -0.343532, 0.512821, -0.269323, 0.965101, 0.506712, -0.854628,
    0.769414, -0.127177, -0.0445365, -0.450186, -0.666986, 0.795313,
    -0.878871, 0.00904579, -0.361934, -0.0120466, -0.818534, -0.852502,
    -0.231716, 0.827635, -0.0711084, -0.899832, 0.540409, -0.749269,
    0.376911, 0.259087, 0.450824, 0.777144, -0.387356, 0.0265474,
    0.691963, 0.683021, -0.169211, -0.0641653, -0.643345, 0.14331,
    -0.933892, -0.00303976, 0.496585, 0.781475, 0.684079, -0.574497,
    -0.739145, -0.450824, -0.171413, 0.419639, -0.520178, -0.364921,
    0.304117, 0.362692, -0.224549, -0.704934, 0.691151, 0.910818,
    -0.703697, -0.182467, 0.129797, -0.0229709, 0.92219, -0.600486,
    0.258538, 0.302507, 0.606146, -0.0471364, -0.593499, 0.803347,
    -0.715958, -0.179374, 0.771297, -0.675603, -0.269322, -0.729781,
    -0.0893854, -0.0953997, 0.863349, -0.569503, 0.817844, 0.72172,
    0.0119118, 0.635123, -0.07551, 0.265477, 0.649395};


  auto knots = Velocity<double>::knots(1.0);

  HorizontalMotion<double> getTrueConstantCurrent() {
    return HorizontalMotion<double>(0.2*knots, -0.3*knots);
  }

  HorizontalMotion<double> current(int i) {
    auto xy = getTrueConstantCurrent();
    auto x = xy[0];
    auto y = xy[1];
    return HorizontalMotion<double>(
        x + (0.01*rng[(i + 20) % rng.size()])*knots,
        y + (0.01*rng[(i + 40) % rng.size()])*knots);
  }


  HorizontalMotion<double> TestSample::corruptedMotionOverWaterVec() const {
    return correctOrCorruptVector(groundTruthMotionOverWaterVec(),
        params[0], params[1], params[2]);
  }


  Array<TestSample> makeTestSamples(Arrayd params) {

    int n = 100;

    double boatSpeed = 4.9;

    Array<TestSample> samples(n);
    for (int i = 0; i < n; i++) {

      auto dir = (i/30) % 2;
      auto angle = dir*0.5*M_PI + 0.1*rng[i % rng.size()];
      auto x = boatSpeed*cos(angle)*knots;
      auto y = boatSpeed*sin(angle)*knots;
      samples[i] = TestSample{
        HorizontalMotion<double>(x, y),
        current(i), params
      };
    }
    return samples;
  }

  Arrayd getDefaultCorruptParams() {
    return Arrayd{1.2, 0.8, 0.2*M_PI};
  }

}
}

