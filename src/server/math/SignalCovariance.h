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
#include <cassert>

namespace sail {
namespace SignalCovariance {

struct Settings {
  Settings() : windowSize(20), maxResidualCount(100) {}
  int windowSize;
  int maxResidualCount;

  int calcWindowPositionCount(int sampleCount) const {
    return sampleCount - windowSize;
  }

  int calcResidualCount(int sampleCount) const {
    return std::min(calcWindowPositionCount(sampleCount), maxResidualCount);
  }
};


template <typename T>
Array<T> elementwiseMul(Array<T> X, Array<T> Y) {
  int n = X.size();
  assert(n == Y.size());
  Array<T> XY(n);
  for (int i = 0; i < n; i++) {
    XY[i] = X[i]*Y[i];
  }
  return XY;
}

template <typename T>
T smoothNonNegAbs(T x, T thresh = 0.001) {
  if (x < 0) {
    return smoothNonNegAbs(-x, thresh);
  } else if (x < thresh) {
    T a = 1.0/(2.0*thresh);
    T b = 0.5*thresh;
    return a*x*x + b;
  }
  return x;
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
 WeightedValue(T w, T v) : weight(w), weightedValue(w*v) {}

 T weight, weightedValue;

 T average() const {
   return weightedValue/weight;
 }

 WeightedValue<T> operator+(const WeightedValue<T> &other) const {
   return WeightedValue<T>{weight + other.weight,
     weightedValue + other.weightedValue};
 }
};

template <typename T>
WeightedValue<T> calcLocalWeightedVariance(Arrayd time,
    Integral1d<T> X, Integral1d<T> X2, int from, int to) {
  T weight = calcSpanWeight(time, from, to);
  auto meanSquaredValue = X2.average(from, to);
  auto meanValue = X.average(from, to);
  return WeightedValue<T>(weight, meanSquaredValue - sqr(meanValue));
}

template <typename T>
T slidingWindowVariance(Arrayd time, Integral1d<T> X, Integral1d<T> X2, int windowSize) {
  WeightedValue<T> sum{0, 0};
  int windowCount = time.size() - windowSize;
  for (int i = 0; i < windowCount; i++) {
    int from = i;
    int to = from + windowSize;
    sum = sum + calcLocalWeightedVariance(time, X, X2, from, to);
  }
  return sum.average();
}

template <typename T>
struct SignalData {
 SignalData(Arrayd times, Array<T> signal, Settings s) :
   itgX(signal), itgX2(elementwiseMul(signal, signal)) {
   variance = slidingWindowVariance(times, itgX, itgX2, s.windowSize);
 }

 int sampleCount() const {
   return time.size();
 }

 Arrayd time;
 Array<T> X;
 Integral1d<T> itgX, itgX2;
 T variance;

 T standardDeviation() const {
   return sqrt(variance);
 }
};

template <typename T>
T calcLocalCovariance(SignalData<T> X, SignalData<T> Y, Integral1d<T> itgXY,
    int from, int to) {
  return itgXY.average(from, to) - X.itgX.average(from, to)*Y.itgX.average(from, to);
}

template <typename T>
void evaluateResiduals(T globalWeight, SignalData<T> X, SignalData<T> Y,
    Settings s, Array<T> *residuals) {
  int n = X.sampleCount();
  auto time = X.time;
  assert(time.identicTo(Y.time));
  int residualCount = s.calcResidualCount(n);
  assert(residualCount == residuals->size());
  int windowCount = s.calcWindowPositionCount(n);
  Integral1d<T> itgXY(elementwiseMul(X.X, Y.X));
  Array<WeightedValue<T> > tmp(windowCount);
  LineKM sampleToResidual(0, windowCount, 0, residualCount);
  for (int i = 0; i < windowCount; i++) {
    int from = i;
    int to = from + s.windowSize;
    T weight = T(calcSpanWeight(time, from, to));
    int index = int(floor(sampleToResidual(i)));
    (*residuals[index]) += weight*sqr(globalWeight*calcLocalCovariance(X, Y, itgXY, from, to));
  }
}










}
}



#endif /* SERVER_MATH_SIGNALCOVARIANCE_H_ */
