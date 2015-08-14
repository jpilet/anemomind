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
  struct LocalCovariance {
    LocalCovariance() : weight(0), sumVarsX(0), sumVarsY(0), sumCovsXY(0) {}
    LocalCovariance(T w, T x, T y, T xy) : weight(w), sumVarsX(x), sumVarsY(y), sumCovsXY(xy) {}
    T weight;
    T sumVarsX, sumVarsY; // For normalization
    T sumCovsXY; // The covariance

    LocalCovariance<T> operator+(const LocalCovariance<T> &other) const {
      return LocalCovariance<T>(
        weight + other.weight,
        sumVarsX + other.sumVarsX,
        sumVarsY + other.sumVarsY,
        sumCovsXY + other.sumCovsXY);
    }

    T stdX() const {
      return sqrt(sumVarsX/regularizedWeight());
    }

    T stdY() const {
      return sqrt(sumVarsY/regularizedWeight());
    }

    T normalizationFactor() const {
      // Add small number to avoid division by zero if one signal is very constant.
      return 1.0/(stdX()*stdY() + 1.0e-9);
    }

    T regularizedWeight() const {
      return abs(weight) + 1.0e-9;
    }
  };

  template <typename T>
  std::ostream &operator<<(std::ostream &s, const LocalCovariance<T> &x) {
    std::cout << "LocalCov (weight=" << x.weight << ", sumVarsX=" << x.sumVarsX
        << ", sumVarsY=" << x.sumVarsY << ", sumCovsXY=" << x.sumCovsXY << ")\n";
        return s;
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
  T calcSumVars(int from, int to, Integral1d<T> itgX, Integral1d<T> itgX2) {
    T n = T(to - from);
    T mu = itgX.average(from, to);
    return itgX2.integrate(from, to) - 2.0*mu*itgX.integrate(from, to) + n*sqr(mu);
  }

  template <typename T>
  T calcSumCovs(int from, int to, Integral1d<T> itgX, Integral1d<T> itgY, Integral1d<T> itgXY) {
    T muX = itgX.average(from, to);
    T muY = itgY.average(from, to);
    T n = T(to - from);
    return itgXY.integrate(from, to) - muY*itgX.integrate(from, to) - muX*itgY.integrate(from, to)
        + n*muX*muY;
  }

  template <typename T>
  LocalCovariance<T> calcLocalCovariance(int from, int to, const Arrayd &time,
      const Integral1d<T> &itgX,
      const Integral1d<T> &itgX2,
      const Integral1d<T> &itgY,
      const Integral1d<T> &itgY2,
      const Integral1d<T> &itgXY) {
    int n = to - from;
    auto w = T(calcSpanWeight(time, from, to));
    return LocalCovariance<T>(
      w,
      w*calcSumVars(from, to, itgX, itgX2),
      w*calcSumVars(from, to, itgY, itgY2),
      w*calcSumCovs(from, to, itgX, itgY, itgXY)
    );
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
  void slidingWindowCovariancesToArray(T globalWeight, SignalData<T> X, SignalData<T> Y,
      Settings s, Array<T> *residuals) {
    int n = X.sampleCount();
    assert(X.time.identicTo(Y.time));
    int residualCount = s.calcResidualCount(n);
    assert(residualCount == dst->size());
    int windowCount = s.calcWindowPositionCount(n);
    Integral1d<T> itgXY(elementwiseMul(X.X, Y.X));
    Array<WeightedValue<T> > tmp(windowCount);
    for (int i = 0; i < windowCount; i++) {
      int from = i;
      int to = from + s.windowSize;

    }
  }



// For use in objective functions where we penalize local covariances using a sliding window.
template <typename T>
LocalCovariance<T> slidingWindowCovariancesToArray(Arrayd time, Array<T> X,
    Array<T> Y, Settings s, Array<T> *dst) {

  int n = time.size();
  assert(n == X.size());
  Integral1d<T> itgX(X);
  Integral1d<T> itgX2(elementwiseMul(X, X));
  Integral1d<T> itgY(Y);
  Integral1d<T> itgY2(elementwiseMul(Y, Y));
  Integral1d<T> itgXY(elementwiseMul(X, Y));


  int windowPositionCount = s.calcWindowPositionCount(n);
  int residualCount = s.calcResidualCount(n);

  assert(dst->size() == residualCount);
  LineKM toResidualIndex(0, windowPositionCount, 0, residualCount);
  LocalCovariance<T> totalSum;
  for (int i = 0; i < windowPositionCount; i++) {
    int from = i;
    int to = from + s.windowSize;
    auto x = calcLocalCovariance(from, to, time, itgX, itgX2, itgY, itgY2, itgXY);
    totalSum = totalSum + x;
    int index = int(floor(toResidualIndex(i)));

    // The covariances of neighboring window positions are accumulated in
    // common residuals, so that the residual vector (and Jacobian) doesn't
    // become too large. Since
    (*dst)[index] += sqr(x.sumCovsXY);
  }

  // Squares it, because we have a sum of squared residuals.
  T f = sqr(totalSum.normalizationFactor());
  for (int i = 0; i < residualCount; i++) {

    // Dirty hack: The sqrt function has derivatives
    // approaching infinity close to 0. So add a positive
    // number to the sum of residuals to make sure that
    // we are not close to zero, before applying the square root.
    // The effect of the square root will be cancelled, because we
    // minimize the sum of squared residuals.
    constexpr double positivityOffset = 1.0e-6;

    (*dst)[i] = sqrt((*dst)[i]*f + positivityOffset);
  }
  return totalSum;
}

template <typename T>
Array<T> slidingWindowCovariances(Arrayd time, Array<T> X,
    Array<T> Y, Settings s) {
  auto dst = Array<T>::fill(s.calcResidualCount(time.size()), T(0.0));
  slidingWindowCovariancesToArray(time, X, Y, s, &dst);
  return dst;
}






}
}



#endif /* SERVER_MATH_SIGNALCOVARIANCE_H_ */
