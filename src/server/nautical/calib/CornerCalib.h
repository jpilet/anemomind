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
#include <server/math/QuadForm.h>

namespace sail {
namespace CornerCalib {

/*
 * Sample is an arbitrary type that represents a set of sampled values.
 * Think of Sample as a type similar to our old Nav class.
 *
 * It must have one method with signature
 *
 * Eigen::Vector2d refMotion() const;
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
Array<T> computeCornerSmoothnesses(
    const Array<PointQuad<T, 2> > &quads, int windowSize) {
  int sampleCount = quads.size();
  typedef PointQuad<T, 2> PQ;
  Integral1d<PQ> itg(quads, PQ());
  int n = sampleCount - windowSize + 1;
  Array<T> dst(n);
  int half = windowSize/2;
  const double mu = 1.0e-12;
  for (int i = 0; i < n; i++) {
    int left = i;
    int middle = left + half;
    int right = left + windowSize;
    dst[i] = 0.5*(itg.integrate(left, middle).computeVariance()
        + itg.integrate(middle, right).computeVariance())
        /(mu + itg.integrate(left, right).computeVariance());
  }
  return dst;
}

template <typename T>
Array<T> computeCornerSmoothnesses(
    const Array<Eigen::Matrix<T, 2, 1> > flows, int windowSize) {
  int sampleCount = flows.size();
  typedef PointQuad<T, 2> PQ;
  Array<PQ> quads(sampleCount);
  for (int i = 0; i < sampleCount; i++) {
    quads[i] = PQ(flows[i]);
  }
  return computeCornerSmoothnesses<T>(quads, windowSize);
}

template <typename Sample, typename TrueFlowFunction>
class Objf {
public:
  Objf(TrueFlowFunction f, const Array<Sample> &samples,
      const Settings &s) : _f(f), _samples(samples),
      _settings(s), _n(samples.size() - s.windowSize + 1) {
    _refSmoothness = computeCornerSmoothnesses<double>(
        sail::map(samples, [&](const Sample &s) {
      return s.refMotion();
    }).toArray(), s.windowSize);
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

    int sampleCount = _samples.size();
    Array<PQ> quads(sampleCount);
    for (int i = 0; i < sampleCount; i++) {
      quads[i] = PQ(_f.template evalFlow<T>(_samples[i], params));
    }
    auto flowSmoothness = computeCornerSmoothnesses<T>(quads, _settings.windowSize);

    typedef QuadForm<2, 1, T> LineForm;
    LineForm lineFit;
    T totalFlowSmoothness = T(0.0);
    for (int i = 0; i < _n; i++) {
      lineFit += LineForm::fitLine(T(_refSmoothness[i]), flowSmoothness[i]);
      totalFlowSmoothness += flowSmoothness[i];
    }
    T meanFlowSmoothness = T(1.0/_n)*totalFlowSmoothness;
    T sumSquaredErrors = lineFit.evalOpt2x1();


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
      residuals[i] = factor*(flowSmoothness[i] - meanFlowSmoothness);
    }

    return true;
  }

  TrueFlowFunction _f;
  Array<Sample> _samples;
  Settings _settings;
  Arrayd _refSmoothness;
  int _n;
};

template <typename Sample, typename TrueFlowFunction>
Arrayd optimizeCornerness(TrueFlowFunction f,
                          const Array<Sample> &samples,
                          const Settings &settings) {

  if (samples.size() < 2*settings.windowSize) {
    LOG(WARNING) << "Too few samples " << (samples.size()) << " w.r.t. window size " <<
        settings.windowSize << " to do anything meaningful. Consider merging sessions"
            " to get more samples.";
    return Arrayd();
  }

  Arrayd params = f.initialParams();
  ceres::Problem problem;
  typedef Objf<Sample, TrueFlowFunction> TObjf;
  auto objf = new TObjf(f, samples, settings);
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
