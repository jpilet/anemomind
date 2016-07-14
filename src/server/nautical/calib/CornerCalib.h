/*
 * CornerCalib.h
 *
 *  Created on: Apr 15, 2016
 *      Author: jonas
 *
 *
 * This file implements the Cornerness-based algorithm, discussed on
 * April 8th 2016. By cornerness, we refer to the corners of a boat trajectory,
 * which are reflected as stepwise transitions in the velocity.
 *
 * This algorithm is implemented with templates, so we can use any parameterization
 * or data we like which satisfy these templates. We can even reuse it for wind later,
 * if we like.
 */

#ifndef SERVER_NAUTICAL_CALIB_CORNERCALIB_H_
#define SERVER_NAUTICAL_CALIB_CORNERCALIB_H_

#include <ceres/ceres.h>
#include <server/common/Array.h>
#include <server/math/PointQuad.h>
#include <server/math/Integral1d.h>
#include <server/common/Functional.h>
#include <server/common/math.h>
#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>

namespace sail {
namespace CornerCalib {

/*
 * Sample is an arbitrary type that represents a set of sampled values.
 * Think of Sample as a type similar to our old Nav class.
 *
 * It must have one method with signature
 *
 * HorizontalMotion<double> refMotion() const;
 *
 * which gives some reference motion, typically the GPS motion.
 *
 *
 *
 *
 * TrueFlowFunction is an arbitrary type with a template method 'evalFlow'
 * with this signature:
 *
 *   template <typename T>
 *   Eigen::Matrix<T, 2, 1> evalFlow(const Sample &s, const T *params) const;
 *
 * and another regular method with this signature:
 *
 *   Arrayd initialParams() const;
 *
 * The 'evalFlow' method will, given calibration parameters 'params', together
 * with a specific sample 's' compute the true flow (wind over ground
 * or sea current over ground). It can compute that any way it likes.
 *
 * The type T will either be a double or some AD type, such as ceres::Jet.
 *
 */
struct Settings {
  int windowSize;
  ceres::Solver::Options ceresOptions;
};

template <typename T>
Velocity<T> getUnit() {return Velocity<T>::knots(T(1.0));}

template <typename T>
Eigen::Matrix<T, 2, 1> motionToVec(const HorizontalMotion<T> &v) {
  return Eigen::Matrix<T, 2, 1>(v[0]/getUnit<T>(), v[1]/getUnit<T>());
}

template <typename T>
HorizontalMotion<T> setNorm(
    const HorizontalMotion<T> &motion,
    Velocity<T> newNorm) {
  Eigen::Matrix<T, 2, 1> v = motionToVec(motion).normalized();
  return HorizontalMotion<T>(v(0)*newNorm, v(1)*newNorm);
}

template <typename T>
void accumulateCornerSmoothnesses(const Array<PointQuad<T, 2> > &quads,
    int windowSize, std::vector<T> *dst) {
  int sampleCount = quads.size();
  typedef PointQuad<T, 2> PQ;
  Integral1d<PQ> itg(quads, PQ());
  int n = sampleCount - windowSize + 1;
  int half = windowSize/2;
  const double mu = 1.0e-12;
  for (int i = 0; i < n; i++) {
    int left = i;
    int middle = left + half;
    int right = left + windowSize;
    dst->push_back(0.5*(itg.integrate(left, middle).computeVariance()
        + itg.integrate(middle, right).computeVariance())
        /(mu + itg.integrate(left, right).computeVariance()));
  }
}

inline int computeQuadCountForGroup(int size, int windowSize) {
  return std::max(size - windowSize + 1, 0);
}

template <typename Sample>
int computeTotalQuadCount(const Array<Array<Sample> > &sampleGroups, int windowSize) {
  int n = 0;
  for (auto g: sampleGroups) {
    n += computeQuadCountForGroup(g.size(), windowSize);
  }
  return n;
}

template <typename Sample, typename TrueFlowFunction>
class CornerObjf {
public:
  CornerObjf(TrueFlowFunction f, const Array<Array<Sample> > &sampleGroups,
      const Settings &s) : _f(f), _sampleGroups(sampleGroups),
      _settings(s), _n(computeTotalQuadCount(sampleGroups, s.windowSize)) {
    _refSmoothness.reserve(_n);
    for (auto samples: _sampleGroups) {
      auto quads = sail::map(samples,
          [&](const Sample &sample) {
        return PointQuad<double, 2>(motionToVec(sample.refMotion()));
      }).toArray();
      accumulateCornerSmoothnesses<double>(quads, s.windowSize, &_refSmoothness);
    }
  }

  int outDims() const {
    return _n;
  }

  template<typename T>
  bool operator()(T const* const* parameters, T* residuals) const {
    return eval(parameters[0], residuals);
  }
private:
  template <typename T>
  bool eval(const T *params, T *residuals) const {
    typedef Eigen::Matrix<T, 2, 1> Vec;
    typedef PointQuad<T, 2> PQ;

    std::vector<T> flowSmoothnesses;
    flowSmoothnesses.reserve(_n);
    for (auto samples: _sampleGroups) {
      auto sampleCount = samples.size();
      Array<PQ> quads(sampleCount);
      for (int i = 0; i < sampleCount; i++) {
        quads[i] = PQ(motionToVec(_f.template evalFlow<T>(samples[i], params)));
      }
      accumulateCornerSmoothnesses(quads, _settings.windowSize, &flowSmoothnesses);
    }
    assert(_n == flowSmoothnesses.size());

    T totalFlowSmoothness = T(0.0);
    T sumSquaredErrors = T(0.0);
    for (int i = 0; i < _n; i++) {
      sumSquaredErrors += sqr(flowSmoothnesses[i] - _refSmoothness[i]);
      totalFlowSmoothness += flowSmoothnesses[i];
    }
    T meanFlowSmoothness = T(1.0/_n)*totalFlowSmoothness;

    /*
     * Explanation: For an ill-calibrated system, we observed that the samples
     * seem to align along a tilted line. So fitting a line to those samples will
     * result in a small fitting error, which is 'sumSquaredErrors'.
     *
     * That in turn, will lead to a large factor, 'factor' below.
     *
     * Then we fit all samples to 'meanFlowSmoothness' which is a horizontal line,
     * weighted by 'factor'. I believe this objective well reflects what we want to
     * do.
     */
    T factor = T(1.0)/sqrt(1.0e-12 + sumSquaredErrors);

    for (int i = 0; i < _n; i++) {
      residuals[i] = factor*(flowSmoothnesses[i] - meanFlowSmoothness);
    }

    return true;
  }

  TrueFlowFunction _f;
  Array<Array<Sample> > _sampleGroups;
  Settings _settings;
  std::vector<double> _refSmoothness;
  int _n;
};

template <typename Sample, typename TrueFlowFunction>
Arrayd optimizeCornernessForGroups(TrueFlowFunction f,
                          const Array<Array<Sample> > &sampleGroups,
                          const Settings &settings) {

  Arrayd params = f.initialParams();
  ceres::Problem problem;
  typedef CornerObjf<Sample, TrueFlowFunction> TObjf;
  auto objf = new TObjf(f, sampleGroups, settings);
  auto cost = new ceres::DynamicAutoDiffCostFunction<TObjf>(objf);
  cost->AddParameterBlock(params.size());
  cost->SetNumResiduals(objf->outDims());
  problem.AddResidualBlock(cost, nullptr, params.ptr());
  ceres::Solver::Summary summary;
  ceres::Solve(settings.ceresOptions, &problem, &summary);
  return params;
}

}
}

#endif /* SERVER_NAUTICAL_CALIB_CORNERCALIB_H_ */
