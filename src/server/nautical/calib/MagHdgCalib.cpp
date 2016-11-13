/*
 * MagHdgCalib.cpp
 *
 *  Created on: 13 Nov 2016
 *      Author: jonas
 */

#include <server/nautical/calib/MagHdgCalib.h>
#include <server/common/TimedValueUtils.h>
#include <ceres/ceres.h>
#include <server/common/string.h>

namespace sail {
namespace MagHdgCalib {

namespace {
  Angle<double> angleUnit = 1.0_rad;
  Velocity<double> velocityUnit = 1.0_mps;

  template <typename T>
  HorizontalMotion<T> toHMotion(const T *xy) {
    return HorizontalMotion<T>(
        xy[0]*velocityUnit.cast<T>(),
        xy[1]*velocityUnit.cast<T>());
  }

  template <typename T>
  Angle<T> toAngle(T x) {
    return x*angleUnit.cast<T>();
  }

  IndexedWindows allocateWindows(
      const Array<TimedValue<Angle<double>>> &samples,
      Duration<double> windowSize) {
    return IndexedWindows(Span<TimeStamp>(
        samples.first().time, samples.last().time),
        windowSize);
  }

  template <typename T>
  using Vec = Eigen::Matrix<T, 2, 1>;

  template <typename T>
  Vec<T> normalize(const Vec<T> &x) {
    return (1.0/(1.0e-9 + x.norm()))*x;
  }

  template <typename T>
  Vec<T> getUnitVector(const HorizontalMotion<T> &x) {
    return normalize(Vec<T>(
        x[0]/velocityUnit.cast<T>(),
        x[1]/velocityUnit.cast<T>()));
  }

  struct MagHdgFitness {
    Angle<double> heading;
    HorizontalMotion<double> gpsMotion;

    template <typename T>
    bool operator()(
        const T *correction1,
        const T *current2,
        T *residuals) const {
      auto current = toHMotion<T>(current2);
      auto correction = toAngle<T>(*correction1);

      HorizontalMotion<T> speedOverWater(
          gpsMotion.cast<T>() - current);

      HorizontalMotion<T> headingMotion = HorizontalMotion<T>::polar(
          velocityUnit, heading.cast<T>() + correction);

      Vec<T> a = getUnitVector(speedOverWater);
      Vec<T> b = getUnitVector(headingMotion);

      Vec<T> c = b - a;
      for (int i = 0; i < 2; i++) {
        residuals[i] = c(i);
      }
      return true;
    }
  };
}

Array<TimedValue<Angle<double>>>
  calibrateSingleChannel(
    SplineGpsFilter::EcefCurve curve,
    const Array<TimedValue<Angle<double>>> &headings,
    const Settings &settings,
    DOM::Node *output) {
  if (headings.size() < 2) {
    DOM::addSubTextNode(output, "p",
        "Not enough data to calibrate magnetic heading");
    return Array<TimedValue<Angle<double>>>();
  }

  IndexedWindows windows = allocateWindows(
      headings, settings.windowSize);

  DOM::addSubTextNode(output, "p",
      stringFormat("Number of windows: %d", windows.size()));

  ceres::Problem problem;

  // Allocate the variables to optimize
  auto localCurrents = Array<Eigen::Vector2d>::fill(
      windows.size(), Eigen::Vector2d::Zero());

  double angleCorrectionRadians = 0.0;
  problem.AddParameterBlock(&angleCorrectionRadians, 1);


  Array<int> checked = Array<int>::fill(windows.size(), 0);
  // Populate the objective function.
  for (auto x: headings) {
    if (curve.covers(x.time)) {
      auto gpsMotion = curve.evaluateHorizontalMotion(x.time);
      auto windowSpan = windows.getWindowIndexSpan(x.time);
      for (auto windowIndex: windowSpan) {
        checked[windowIndex]++;
        problem.AddResidualBlock(
            new ceres::AutoDiffCostFunction<
              MagHdgFitness, 2, 1, 2>(new MagHdgFitness{
                  x.value, gpsMotion}),
            nullptr,
            &angleCorrectionRadians,
            localCurrents[windowIndex].data());
      }
    }
  }


  std::vector<int> missing;
  Span<int> v;
  for (int i = 0; i < localCurrents.size(); i++) {
    int n = checked[i];
    v.extend(n);
    if (0 < n) {
      problem.AddParameterBlock(localCurrents[i].data(), 2);
    } else {
      missing.push_back(i);
    }
  }{
    DOM::addSubTextNode(output, "p",
        stringFormat("Minimum samples per window: %d",
            v.minv()));
    DOM::addSubTextNode(output, "p",
        stringFormat("Maximum samples per window: %d", v.maxv()));
    if (!missing.empty()) {
      std::stringstream ss;
      ss << "The following windows were not covered: ";
      for (auto i: missing) {
        ss << i << " ";
      }
      DOM::addSubTextNode(output, "p", ss.str());
    }
  }

  ceres::Solver::Options options;
  ceres::Solver::Summary summary;
  ceres::Solve(options, &problem, &summary);
  {
    addSubTextNode(output, "pre", summary.BriefReport());
  }

  auto correction = toAngle<double>(angleCorrectionRadians);

  DOM::addSubTextNode(output, "p",
      stringFormat("Angle correction: %.3g deg",
          correction.degrees()));

  return Array<TimedValue<Angle<double>>>();
}

}
}
