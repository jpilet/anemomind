/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "ManoeuverDetector.h"
#include <server/math/nonlinear/GeneralizedTV.h>
#include <server/math/CleanNumArray.h>

namespace sail {

ManoeuverDetector::ManoeuverDetector() :
  _regWeight(1000), _minManoeuverCount(8),
  _initManoeuverThresh(2.0), _stableThresh(0.1) {}

namespace {
  bool extractManoeuvers(Arrayd fx, Arrayd fy,
      int minCount, double thresh, double stableThresh,
      std::vector<ManoeuverDetector::Manoeuver> *dst) {
    dst->resize(0);
    if (thresh <= stableThresh) {
      return false;
    } else {
      int from = -1;
      int count = fy.size();
      bool lastStable = std::abs(fy[0]) < stableThresh;
      bool detected = false;
      double strength = 0;
      int peakLoc = 0;
      for (int i = 1; i < count; i++) {
        double x = std::abs(fy[i]);
        bool currentStable = x < stableThresh;
        if (currentStable) {
          if (detected && from != -1) {
            dst->push_back(
                ManoeuverDetector::Manoeuver(strength,
                    Duration<double>::seconds(fx[peakLoc]),
                    Duration<double>::seconds(fx[from]),
                    Duration<double>::seconds(fx[i])));
          }

          detected = false;
          strength = 0;
          from = i;
        } else if (thresh < x) {
          detected = true;
          if (x > strength) {
            strength = x;
            peakLoc = i;
          }
        }

        lastStable = currentStable;
      }

      if (dst->size() >= minCount) {
        return true;
      } else {
        return extractManoeuvers(fx, fy, minCount, 0.5*thresh, stableThresh, dst);
      }
    }
  }

}

Array<ManoeuverDetector::Manoeuver> ManoeuverDetector::detect(Array<Duration<double> > times,
    Array<Angle<double> > angles) const {
  GeneralizedTV tv;
  Arrayd X = times.map<double>([](Duration<double> x) {return x.seconds();});
  Arrayd Y = cleanContinuousAngles(angles).map<double>([](Angle<double> d) {return d.degrees();});
  UniformSamples filteredResult = tv.filter(X, Y, 1.0, 1, _regWeight);
  Arrayd fx = filteredResult.makeCentredX();
  Arrayd fy = filteredResult.interpolateLinearDerivative(fx);
  std::vector<Manoeuver> dst;
  dst.reserve(_minManoeuverCount);
  if (extractManoeuvers(fx, fy, _minManoeuverCount, _initManoeuverThresh, _stableThresh,
      &dst)) {
      return Array<Manoeuver>::makeArrayCopy(dst);
  } else {
    return Array<Manoeuver>();
  }
}


}
