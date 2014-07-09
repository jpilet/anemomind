/*
 * 1) Load races
 * 2) Construct a Ceres optimization problem
 * 3) Solve it
 * 4) Load Test data
 * 5) Check optimized data with test data.
 */

#include "Calibrator.h"

#include <ceres/ceres.h>
#include <cmath>
#include <device/Arduino/libraries/ChunkFile/ChunkFile.h>
#include <device/Arduino/libraries/FixedPoint/FixedPoint.h>
#include <device/Arduino/libraries/TrueWindEstimator/InstrumentFilter.h>
#include <iostream>
#include <server/common/math.h>
#include <server/common/string.h>
#include <server/nautical/NavNmeaScan.h>
#include <string>
#include <server/plot/extra.h>
#include <server/common/Histogram.h>
#include <server/common/ArrayBuilder.h>

using ceres::AutoDiffCostFunction;
using ceres::CostFunction;
using ceres::Problem;
using ceres::Solver;
using ceres::Solve;
using std::string;

namespace sail {

namespace {

string showWind(const HorizontalMotion<double>& wind) {
  double degrees = wind.angle().degrees();
  return stringFormat("%3.0f/%4.1fkn",
                      positiveMod(degrees, 360.0),
                      wind.norm().knots());
}

Angle<double> externalTrueWindDirection(Array<Nav> nav) {
  return nav.last().externalTwa() + nav.last().magHdg();
}

template <typename T, int N>
bool isnan(const ceres::Jet<T, N>& f) { return ceres::IsNaN(f); }

using std::isnan;

}  // namespace

class TackCost {
  public:
    TackCost(Array<Nav> before, Array<Nav> after, double weight)
      : _before(makeFilter(before)), _after(makeFilter(after)),
      _beforeNav(before), _afterNav(after), _weight(weight) { }

    template<typename T>
    bool operator()(const T* const x, T* residual) const {
      HorizontalMotion<T> windBefore =
        TrueWindEstimator::computeTrueWind<T, ServerFilter>(x, _before);
      HorizontalMotion<T> windAfter =
        TrueWindEstimator::computeTrueWind<T, ServerFilter>(x, _after);

      if (1) {
        HorizontalMotion<T> difference = windAfter - windBefore;
        residual[0] = T(_weight) * difference[0].knots();
        residual[1] = T(_weight) * difference[1].knots();
      } else {
        residual[0] = T(_weight) * (windAfter.angle().directionDifference(
                windBefore.angle()).degrees());

        T strengthBefore = windBefore.norm().metersPerSecond();
        T strengthAfter = windAfter.norm().metersPerSecond();

        // The strength has to be normalized. Otherwise, the optimizer will
        // be rewarded for underestimating the wind.
        residual[1] = T(_weight * 360.0 * 2.0) * (
            (strengthAfter - strengthBefore) / (strengthAfter));
      }
      assert(!isnan(residual[0]));
      assert(!isnan(residual[1]));
      return true;
    }

    void printCost(const double* params) {
      HorizontalMotion<double> windBefore =
        TrueWindEstimator::computeTrueWind<double, ServerFilter>(params, _before);
      HorizontalMotion<double> windAfter =
        TrueWindEstimator::computeTrueWind<double, ServerFilter>(params, _after);
      std::cout << "Wind: " << showWind(windBefore) << " and "
        << showWind(windAfter)
        << " at " << _before.oldestUpdate().toString()
        << " and " << _after.oldestUpdate().toString()
        << " w=" << _weight << "\n";
    }

    void angularError(const double *params,
                         double *sumDegrees, double *sumKnots,
                         double *sumExternalDegrees, double *sumExternalKnots) {
      HorizontalMotion<double> windBefore =
        TrueWindEstimator::computeTrueWind<double, ServerFilter>(params, _before);
      HorizontalMotion<double> windAfter =
        TrueWindEstimator::computeTrueWind<double, ServerFilter>(params, _after);
      *sumDegrees = std::fabs(windAfter.angle().directionDifference(
              windBefore.angle()).degrees());
      *sumKnots = std::fabs((windAfter.norm() - windBefore.norm()).knots());
      
      *sumExternalDegrees = std::fabs(
          externalTrueWindDirection(_beforeNav).directionDifference(
              externalTrueWindDirection(_afterNav)).degrees());
      *sumExternalKnots = std::fabs(
          (_beforeNav.last().externalTws() - _afterNav.last().externalTws()).knots());
    }

  private:
    ServerFilter _before;
    ServerFilter _after;

    // For debugging only.
    Array<Nav> _beforeNav;
    Array<Nav> _afterNav;
    double _weight;
};

void Calibrator::addTack(int pos, double weight) {
  const int length = 5;
  const int delta = 20;
  if ((pos < length + delta) || (pos > _allnavs.size() - length - delta)) {
    LOG(WARNING) << "Ignoring maneuver too close to beginning or end of recording";
    return;
  }

  const int largeMargin = 500;
  Array<Nav> before = _allnavs.slice(max(0, pos - delta - largeMargin),
                                     pos - delta);
  Array<Nav> after = _allnavs.slice(max(0, pos + delta - largeMargin),
                                    pos + delta + length);

  Duration<double> deltaTime = after.first().time() - before.last().time();
  if (deltaTime > Duration<>::minutes(3)) {
    LOG(WARNING) << "Ignoring maneuver with a long time gap.";
    return;
  }

  const Velocity<double> minWindSpeed = Velocity<double>::knots(2.0);
  if (after.last().aws() < minWindSpeed ||
      before.last().aws() < minWindSpeed) {
    // less than 2 knots of wind is not a useful measure.
    return;
  }
  
  if (fabs(externalTrueWindDirection(before).directionDifference(
          externalTrueWindDirection(after)).degrees()) > 30.0) {
    // Tacktick is off by more than 30 degrees: something funny is going on.
    return;
  }

  TackCost *cost = new TackCost(before, after, weight);
  _maneuvers.push_back(cost);
  CostFunction* cost_function =
      new AutoDiffCostFunction<
          TackCost,
          2, //residuals
          TrueWindEstimator::NUM_PARAMS // unknowns
        >(cost);
  _problem.AddResidualBlock(cost_function,
                            //nullptr,
                            new ceres::CauchyLoss(1),
                            _calibrationValues);
}

void Calibrator::addBuoyTurn(std::shared_ptr<HTree> tree) {
    for (int i = 0; i < tree->childCount() - 1; ++i) {
      auto before = tree->child(i);
      auto after = tree->child(i + 1);
      std::string beforeDescr = description(before);
      std::string afterDescr = description(after);
      if ((beforeDescr == "downwind-leg" && afterDescr == "upwind-leg")
          ||(beforeDescr == "upwind-leg" && afterDescr == "downwind-leg")) {
        addTack(before->right(), 10.0);
      }
    }
    for (auto child : tree->children()) {
      addBuoyTurn(child);
    }
}

void Calibrator::addAllTack(std::shared_ptr<HTree> tree) {
  if (description(tree) == "upwind-leg" ||
      description(tree) == "downwind-leg") {
    for (int i = 0; i < tree->childCount() - 1; ++i) {
      auto before = tree->child(i);
      auto after = tree->child(i + 1);
      std::string beforeDescr = description(before);
      std::string afterDescr = description(after);
      if ((beforeDescr == "starboard-tack" && afterDescr == "port-tack")
          || (beforeDescr == "port-tack" && afterDescr == "starboard-tack")) {
        // We have a maneuver.
        /*
        LOG(INFO) << "Tack: " << before->right() << " - "
          << after->left() << ": "
          << description(before->child(before->childCount() - 1))
          << "(" << before->child(before->childCount() - 1)->count() << ")"
          << " -> "
          << description(after->child(0))
          << "(" << after->child(0)->count() << ")";
          */
        std::string childBeforeDescr = description(before->child(before->childCount() - 1));
        std::string childAfterDescr = description(after->child(0));

        if (childBeforeDescr == childAfterDescr
            && (childBeforeDescr == "close-hauled" || childBeforeDescr == "broad-reach")
            && (before->child(before->childCount() - 1)->count() > 20)
            && (after->child(0)->count() > 20)) {
          addTack(before->right(), 1.0);
        }
      }
    }
  } else {
    for (auto child : tree->children()) {
      addAllTack(child);
    }
  }
}

string Calibrator::description(std::shared_ptr<HTree> tree) {
  return _grammar.nodeInfo()[tree->index()].description();
}

bool Calibrator::calibrate(Poco::Path dataPath, Nav::Id boatId) {
  // Load data.
  Array<Nav> allnavs = scanNmeaFolder(dataPath, boatId);
  if (allnavs.size() == 0) {
    return false;
  }

  std::shared_ptr<HTree> tree = _grammar.parse(allnavs);

  return calibrate(allnavs, tree, boatId);
}

bool Calibrator::calibrate(const Array<Nav>& navs,
                           std::shared_ptr<HTree> tree,
                           Nav::Id boatId) {
  clear();
  _tree = tree;
  _allnavs = navs;
  addAllTack(_tree);
  addBuoyTurn(_tree);

  if (_maneuvers.size() < 30) {
    // we do not have enough maneuvers.
    if (_verbose) {
      LOG(INFO) << "only " << _maneuvers.size()
        << " maneuvers, cancelling calibration.";
    }
    clear();
    return false;
  }

  GnuplotExtra gnuplot;
  if (_verbose) {
    print();
    gnuplot.set_style("lines");
    gnuplot.set_xlabel("error [degrees]");
    gnuplot.set_ylabel("count");
    plot(&gnuplot, "external", true);
    plot(&gnuplot, "before", false);
  }

  // Run the solver!
  Solver::Options options;
  options.minimizer_progress_to_stdout = true;
  options.max_num_iterations = 500;
  options.function_tolerance = 1e-7;
  options.minimizer_progress_to_stdout = _verbose;
  Solver::Summary summary;
  Solve(options, &_problem, &summary);

  if (_verbose) {
    std::cout << summary.BriefReport() << "\n";

    for (int i = 0; i < TrueWindEstimator::NUM_PARAMS; ++i) {
      LOG(INFO) << "param: " << _calibrationValues[i];
    }

    print();
    plot(&gnuplot, "after", false);
  }

  return true;
}

void Calibrator::saveCalibration(std::ofstream *file) {
  TrueWindEstimator::Parameters<FP16_16> calibration;
  for (int i = 0; i < TrueWindEstimator::NUM_PARAMS; ++i) {
    // Convert to fixed point.
    calibration.params[i] = _calibrationValues[i];
  }
  writeChunk(*file, &calibration);
}

void Calibrator::print() {
  double sumAngleError = 0;
  double sumNormAngle = 0;
  double sumExternalAngleError = 0;
  double sumExternalNormAngle = 0;
  ArrayBuilder<double> allAngleError; 

  for (auto maneuver : _maneuvers) {
    if (_maneuvers.size() < 30) {  // Avoid flooding.
      maneuver->printCost(_calibrationValues);
    }
    double angleError = 0;
    double normAngle = 0;
    double externalAngleError = 0;
    double externalNormAngle = 0;
    maneuver->angularError(_calibrationValues,
                              &angleError, &normAngle,
                              &externalAngleError, &externalNormAngle);

    allAngleError.add(angleError);
    sumAngleError += angleError;
    sumNormAngle += normAngle;
    sumExternalAngleError += externalAngleError;
    sumExternalNormAngle += externalNormAngle;

  }

  sumAngleError /= _maneuvers.size();
  sumNormAngle /= _maneuvers.size();
  sumExternalAngleError /= _maneuvers.size();
  sumExternalNormAngle /= _maneuvers.size();

  LOG(INFO) << "\n * Average angle error: " << sumAngleError << " degrees"
    << " (external: " << sumExternalAngleError << ")\n"
    << " * Average speed error: " << sumNormAngle << " knots"
    << " (external: " << sumExternalNormAngle << ")";
}

void Calibrator::plot(GnuplotExtra *gnuplot, const std::string &title, bool external) {
  ArrayBuilder<double> allAngleError; 
  ArrayBuilder<double> allNormAngle;
  ArrayBuilder<double> allExternalAngleError;
  ArrayBuilder<double> allExternalNormAngle;

  for (auto maneuver : _maneuvers) {
    double angleError = 0;
    double normAngle = 0;
    double externalAngleError = 0;
    double externalNormAngle = 0;
    maneuver->angularError(_calibrationValues,
                              &angleError, &normAngle,
                              &externalAngleError, &externalNormAngle);
    allAngleError.add(angleError);
    allNormAngle.add(normAngle);
    allExternalAngleError.add(externalAngleError);
    allExternalNormAngle.add(externalNormAngle);
  }
  const int count = 16;
  HistogramMap angleErrorHist(count, 0, 64);
  gnuplot->plot(
      angleErrorHist.makePlotData(
          angleErrorHist.countPerBin(
              (external ? allExternalAngleError : allAngleError).get())),
      title
      );
}

void Calibrator::clear() {
  _allnavs.clear();
  _tree.reset();

  TrueWindEstimator::initializeParameters(_calibrationValues);

  _maneuvers.clear();
}

}  // namespace sail

