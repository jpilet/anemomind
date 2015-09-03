/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef SERVER_MATH_SIGNALCOVARIANCE_H_
#define SERVER_MATH_SIGNALCOVARIANCE_H_

#include <server/math/Integral1d.h>
#include <server/common/LineKM.h>
#include <server/common/math.h>
#include <cassert>
#include <iostream>
#include <server/common/ScopedLog.h>
#include <server/common/Progress.h>
#include <server/common/ArrayIO.h>
#include <server/common/string.h>
#include <server/common/math.h>
#include <cassert>


namespace sail {
namespace SignalCovariance {

template <typename T>
bool isReasonable(T x) {
  auto maxv = T(1.0e20);
  return -maxv < x && x < maxv;
}

template <typename T>
bool hasNan(Array<T> X) {
  for (auto x: X) {
    if (genericIsNan(x)) {
      return true;
    }
  }
  return false;
}

struct Settings {
  Settings() : windowSize(30), maxResidualCount(100), absThresh(1.0e-6) {}
  int windowSize;
  int maxResidualCount;
  double absThresh;

  int calcWindowPositionCount(int sampleCount) const {
    return sampleCount - windowSize;
  }

  int calcResidualCount(int sampleCount) const {
    return std::min(calcWindowPositionCount(sampleCount), maxResidualCount);
  }

  template <typename T>
  T abs(T x) const {
    return smoothNonNegAbs2(x, T(absThresh));
  }

  template <typename T>
  T calcWeight(T xVar, T yVar) const {
    return 1.0/sqrt(this->abs(xVar*yVar));
  }
};


template <typename T, typename T2>
Array<T> elementwiseMul(Array<T> X, Array<T2> Y) {
  int n = X.size();
  if (!(n == Y.size())) {
    std::cout << EXPR_AND_VAL_AS_STRING(n) << std::endl;
    std::cout << EXPR_AND_VAL_AS_STRING(Y.size()) << std::endl;
  }
  assert(n == Y.size());
  Array<T> XY(n);
  for (int i = 0; i < n; i++) {
    XY[i] = X[i]*Y[i];
  }
  return XY;
}

/*
 * The shorter the time span across which we estimate covariance and vars,
 * the greater the density of the samples. We give more weight to window
 * positions with greater sampling density.
 */
inline double calcSpanWeight(const Arrayd &time, int from, int to) {
  return 1.0/(time[to] - time[from]);
}

template <typename T>
struct WeightedValue {
 WeightedValue() : weight(T(0.0)), weightedValue(T(0.0)) {}
 WeightedValue(T w, T v) : weight(w), weightedValue(v) {}

 static WeightedValue<T> weighted(T w, T v) {
   return WeightedValue<T>(w, w*v);
 }

 T weight, weightedValue;

 T average() const {
   return weightedValue/weight;
 }

 WeightedValue<T> operator+(const WeightedValue<T> &other) const {
   return WeightedValue<T>(weight + other.weight,
     weightedValue + other.weightedValue);
 }
};

template <typename T>
WeightedValue<T> calcLocalWeightedVariance(Arrayd time,
    Integral1d<T> X, Integral1d<T> X2, int from, int to) {
  T weight = T(calcSpanWeight(time, from, to));
  auto meanSquaredValue = X2.average(from, to);
  auto meanValue = X.average(from, to);
  return WeightedValue<T>::weighted(weight, meanSquaredValue - sqr(meanValue));
}

template <typename T>
T slidingWindowVariance(Arrayd time, Integral1d<T> X, Integral1d<T> X2, int windowSize) {
  WeightedValue<T> sum{T(0), T(0)};
  int windowCount = time.size() - windowSize;

  auto maxWeight = T(0);
  auto maxWeightedValue = T(0);
  for (int i = 0; i < windowCount; i++) {
    int from = i;
    int to = from + windowSize;

    auto tmp = calcLocalWeightedVariance(time, X, X2, from, to);
    sum = sum + tmp;
  }
  return sum.average();
}

template <typename T>
struct SignalData {
 SignalData(Arrayd times, Array<T> signal, Settings s) :
   time(times), X(signal), itgX(signal), _variance(-1),
   _windowSize(s.windowSize) {
   assert(!hasNan(X));
 }

 int sampleCount() const {
   return time.size();
 }

 Arrayd time;
 Array<T> X;
 Integral1d<T> itgX;


 T standardDeviation() {
   return sqrt(variance());
 }

 T variance() {
   if (_variance < T(0)) {
     _itgX2 = Integral1d<T>(elementwiseMul(X, X));
     _variance = slidingWindowVariance(time, itgX, _itgX2, _windowSize);
   }
   return _variance;
 }
 private:
  Integral1d<T> _itgX2;
  int _windowSize;
  T _variance;
};

template <typename T, typename T2>
T calcLocalCovariance(SignalData<T> X, SignalData<T2> Y, Integral1d<T> itgXY,
    int from, int to) {
  // See https://en.wikipedia.org/wiki/Covariance :
  //   Cov(X, Y) = E[XY] - E[X]E[Y]
  assert(itgXY.size() == X.itgX.size());
  assert(itgXY.size() == Y.itgX.size());
  assert(from < to);
  assert(0 <= from);
  assert(to <= itgXY.size());
  auto xy = itgXY.average(from, to);
  auto x = X.itgX.average(from, to);
  auto y = Y.itgX.average(from, to);
  assert(!genericIsNan(xy));
  assert(!genericIsNan(x));
  assert(!genericIsNan(y));
  return xy - x*y;
}

template <typename T>
T calcGlobalWeight(const SignalData<T> &X, const SignalData<T> &Y, Settings s) {
  return 1.0/sqrt(s.abs(X.variance*Y.variance));
}

template <typename T, typename T2>
void evaluateResiduals(T globalWeight, // The global weight can be 1.0/(sigmaX*sigmaY) for normalization.
    SignalData<T> X, SignalData<T2> Y,
    Settings s, Array<T> *residuals) {
  int n = X.sampleCount();
  auto time = X.time;
  assert(time.identicTo(Y.time));
  int residualCount = s.calcResidualCount(n);

  Integral1d<T> itgXY(elementwiseMul(X.X, Y.X));

  assert(residualCount == residuals->size());

  int windowCount = s.calcWindowPositionCount(n);
  Array<WeightedValue<T> > tmp(windowCount);
  LineKM sampleToResidual(0, windowCount, 0, residualCount);
  residuals->setTo(T(0.0));
  for (int i = 0; i < windowCount; i++) {
    int from = i;
    int to = from + s.windowSize;
    T weight = T(calcSpanWeight(time, from, to));
    assert(!genericIsNan(weight));
    int index = int(floor(sampleToResidual(i)));
    assert(!genericIsNan(weight));
    auto cov = calcLocalCovariance(X, Y, itgXY, from, to);
    assert(!genericIsNan(cov));
    (*residuals)[index] += weight*s.abs(cov);
  }
  for (int i = 0; i < residualCount; i++) {
    T &r = (*residuals)[i];
    assert(!genericIsNan(r));
    r = sqrt(globalWeight*r);
    assert(!genericIsNan(r));
  }
}

}
}



#endif /* SERVER_MATH_SIGNALCOVARIANCE_H_ */
