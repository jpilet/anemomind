/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <cassert>
#include <server/nautical/tgtspeed/TargetSpeedPoint.h>
#include <server/common/Span.h>
#include <server/common/Functional.h>

namespace sail {

template <typename T>
struct ValueWithStability {
  ValueWithStability() : stability(-1) {}
  ValueWithStability(T v, double s) : value(v), stability(s) {}
  T value;
  double stability;
};

template <typename T>
struct ValueWithOrder {
  ValueWithOrder() : order(-1) {}
  ValueWithOrder(T a, T b, int o) : value(0.5*(a + b)), dif(fabs(a - b)), order(o) {}
  T value;
  T dif;
  int order;
  bool operator< (const ValueWithOrder &other) const {
    return dif < other.dif;
  }
};

template <typename T>
Array<ValueWithStability<T> > computeValuesWithStability(Array<T> values) {
  int n = values.size() - 1;
  int lastIndex = n - 1;
  Array<ValueWithOrder<T> > valuesWithOrder(n);
  for (int i = 0; i < n; i++) {
    valuesWithOrder[i] = ValueWithOrder<T>(values[i], values[i+1], i);
  }
  std::sort(valuesWithOrder.begin(), valuesWithOrder.end());
  Array<ValueWithStability<T> > dst(n);
  for (int i = 0; i < n; i++) {
    auto x = valuesWithOrder[i];
    ValueWithStability<T> y(x.value, 1.0 - double(i)/lastIndex);
    dst[x.order] = y;
  }
  return dst;
}




Array<Velocity<double> > getBoatSpeeds(Array<TargetSpeedPoint> x) {
  return toArray(map(x, [&](const TargetSpeedPoint &x) {return x.boatSpeed();}));
}
Array<Velocity<double> > getWindSpeeds(Array<TargetSpeedPoint> x) {
  return toArray(map(x, [&](const TargetSpeedPoint &x) {return x.windSpeed();}));
}

Array<Angle<double> > getWindAngles(Array<TargetSpeedPoint> x) {
  return toArray(map(x, [&](const TargetSpeedPoint &x) {return x.windAngle();}));
}

Array<double> getStabilities(Array<TargetSpeedPoint> x) {
  return toArray(map(x, [&] (const TargetSpeedPoint &x) {return x.stability()();}));
}

Array<TargetSpeedPoint> computeStabilities(Array<TargetSpeedPoint> src) {
  auto boatSpeeds = computeValuesWithStability(getBoatSpeeds(src));
  auto windSpeeds = computeValuesWithStability(getWindSpeeds(src));
  auto windAngles = computeValuesWithStability(getWindAngles(src));
  auto n = boatSpeeds.size();
  assert(n == windSpeeds.size());
  assert(n == windAngles.size());
  Array<TargetSpeedPoint> dst(n);
  for (auto index : Spani(0, n)) {
    auto boatSpeed = boatSpeeds[index];
    auto windSpeed = windSpeeds[index];
    auto windAngle = windAngles[index];

    // Fuzzy-logic inspired: if the stability is a truthiness-score between 0 and 1,
    // then 'and' corresponds to the 'min' function. 1 means stable, 0 means not stable.
    // So in order for a PolarNavData to be stable, the boatSpeed, windSpeed and windAngle
    // should also be stable.
    double stability = std::min(boatSpeed.stability, std::min(windSpeed.stability, windAngle.stability));

    dst[index] = TargetSpeedPoint(boatSpeed.value, windSpeed.value, windAngle.value, stability);
  }
  return dst;
}

// Make the stabilities uniformly distributed between 0 and 1.
// Obs: will reorder the data points
Array<TargetSpeedPoint> normalizeStabilities(Array<TargetSpeedPoint> src) {
  Array<TargetSpeedPoint> dst = src.dup();
  class Cmp {
   public:
     bool operator() (const TargetSpeedPoint &a, const TargetSpeedPoint &b) const {
       return a.stability()() < b.stability()();
     }
  };

  std::sort(dst.begin(), dst.end(), Cmp());
  int n = dst.size();
  int lastIndex = n - 1;
  for (int i = 0; i < n; i++) {
    dst[i].setStability(double(i)/lastIndex);
  }
  return dst;
}



}

