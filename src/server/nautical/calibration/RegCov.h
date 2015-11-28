/*
 *  Created on: 2015
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#ifndef SERVER_NAUTICAL_CALIBRATION_REGCOV_H_
#define SERVER_NAUTICAL_CALIBRATION_REGCOV_H_

#include <server/common/Array.h>
#include <server/common/logging.h>
#include <server/common/Functional.h>
#include <server/common/Span.h>
#include <server/common/math.h>
#include <iostream>
#include <server/common/string.h>
#include <server/math/CeresUtils.h>

namespace sail {
namespace RegCov {

int getDataCount(int dim);

template <typename T>
Array<T> accumulateTrajectory(Array<T> src) {
  const int dim = 2;
  int n = src.size()/dim;
  CHECK(n*dim == src.size());
  auto dst = Array<T>::fill((n + 1)*dim, T(0.0));
  for (int i = 0; i < n; i++) {
    int srcOffset = dim*i;
    int dstOffset = srcOffset + dim;
    for (int j = 0; j < dim; j++) {
      dst[dstOffset + j] = dst[dstOffset + j - dim] + src[srcOffset + j];
    }
  }
  return dst;
}

template <typename T>
T calcReg(Array<T> trajectory, int index, int step, double reg = 1.0e-9) {
  int dim = 2;
  int dimStep = dim*step;
  int a = dim*index;
  int b = a + dimStep;
  int c = b + dimStep;
  T sum(0.0);
  for (int i = 0; i < 2; i++) {
    sum += sqr(trajectory[a + i] - 2.0*trajectory[b + i] + trajectory[c + i]);
  }
  return sqrt(sum + reg);
}

template <typename T>
Mapped<T> computeRegs(Array<T> src, int step) {
  auto acc = accumulateTrajectory(src);
  int n = acc.size()/2;
  CHECK(2*n == acc.size());
  int regCount = std::max(0, n - 2*step);
  return map(Spani(0, regCount), [=](int i) {
    return calcReg(acc, i, step);
  });
}

int computeDifCount(int dataSize, int step);

template <typename T>
Mapped<T> computeDifs(const Mapped<T> &src) {
  return map(Spani(0, std::max(0, src.size()-1)), [=](int i) {
    return src[i+1] - src[i];
  });
}

template <typename T>
Mapped<T> computeRegDifs(Array<T> src, int step) {
  return computeDifs(computeRegs(src, step));
}

template <typename T>
T computeMean(Mapped<T> x) {
  return (1.0/x.size())*reduce(x, [](T x, T y) {
    return x + y;
  });
}

template <typename T>
Mapped<T> subtractMean(Mapped<T> X) {
  auto mean = computeMean(X);
  return map(X, [=](T x) {return x - mean;});
}

template <typename T> // Well, not really covariance. We don't divide by number of samples.
T computeCovariance(Arrayd gpsDifs, Array<T> flowDifs, Arrayi split) {
  return map(subtractMean(subsetByIndex(gpsDifs, split)),
             subtractMean(subsetByIndex(flowDifs, split)),
             [](double x, T y) {
               return T(x*y);
             })
         .reduce([=](T x, T y) {return x + y;});
}

template <typename T>
Mapped<T> computeCovariances(Arrayd gpsDifs, Array<T> flowDifs,
    Array<Arrayi> splits) {
  return map(splits, [=](Arrayi split) {
    return computeCovariance(gpsDifs, flowDifs, split);
  });
}

struct Settings {
 int step = 100;
 int samplesPerSplit = 100;
 ceres::Solver::Options ceresOptions;
};

int getMaxIndex(Array<Arrayi> splits);

template <typename TrueFlowFunction>
class Objf {
 public:
  Objf(TrueFlowFunction f, Arrayd gpsSpeeds, Array<Arrayi> splits,
    Settings settings, int parameterCount)
    : _trueFlow(f), _gpsDifs(computeRegDifs(gpsSpeeds, settings.step)),
      _splits(splits), _settings(settings),
      _parameterCount(parameterCount) {
    CHECK(getMaxIndex(splits) < _gpsDifs.size());
  }

  template <typename T>
  bool eval(const T *parameters, T *residuals) {
    auto flowDifs = computeRegDifs(_trueFlow(parameters), _settings.step).toArray();
    CHECK(flowDifs.size() == _gpsDifs.size());
    computeCovariances(_gpsDifs, flowDifs, _splits).putInArray(residuals);
  }

  int inDims() const {
    return _parameterCount;
  }

  int outDims() const {
    return _splits.size();
  }
 private:
  TrueFlowFunction _trueFlow;
  Arrayd _gpsDifs;
  Array<Arrayi> _splits;
  Settings _settings;
  int _parameterCount;
};

struct Summary {
  // Redundant data, used for plotting etc.
  Settings settings;
  Array<Arrayi> splits;
  Arrayd gpsSpeed;
  Arrayd initialFlow;
  Arrayd finalFlow;
  Arrayd initialX;

  // The actual result
  Arrayd finalX;

  void plot() const;
};

/*
 * TrueFlowFunction:
 *
 * A function-like object (instance of a class with the () operator for type T)
 * that accepts a
 * pointer to an array of length initialParameters.size()
 * with numbers of some type T.
 *
 * Should return array on the format of (x0, y0, x1, y1, .... )
 * of corrected flows.
 */
template <typename TrueFlowFunction>
Summary optimize(TrueFlowFunction flow,
                Arrayd gpsSpeeds,
                Array<Arrayi> splits,
                Arrayd initialParameters,
                Settings settings) {
  auto objf = std::shared_ptr<Objf<TrueFlowFunction> >(
      new Objf<TrueFlowFunction>(flow, gpsSpeeds, splits, settings, initialParameters.size()));
  Arrayd X = initialParameters.dup();
  ceres::Problem problem;
  CeresUtils::configureSingleObjfAndParameterBlock(
      &problem, objf, X.ptr());
  ceres::Solver::Summary summary;
  Solve(settings.ceresOptions, &problem, &summary);
  return Summary{
    settings,
    splits,
    gpsSpeeds,
    flow(initialParameters.ptr()),
    flow(X.ptr()),
    initialParameters,
    X
  };
}

Summary optimizeLinear(Eigen::MatrixXd A,
                       Eigen::VectorXd B,
                       Array<Arrayi> splits,
                       Arrayd initialParameters,
                       Settings settings);

}
}

#endif /* SERVER_NAUTICAL_CALIBRATION_REGCOV_H_ */
