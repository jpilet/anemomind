/*
 * 1) Load races
 * 2) Construct a Ceres optimization problem
 * 3) Solve it
 * 4) Load Test data
 * 5) Check optimized data with test data.
 */

#include "Calibrator.h"

#include <server/nautical/NavNmeaScan.h>
#include <ceres/ceres.h>
#include <string>
#include <iostream>
#include <server/common/string.h>

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
                      (degrees < 0 ? degrees + 360 : degrees),
                      wind.norm().knots());
}

}  // namespace

class TackCost {
  public:
    TackCost(Array<Nav> before, Array<Nav> after, double weight)
      : _before(before), _after(after), _weight(weight) { }

    template<typename T>
    bool operator()(const T* const x, T* residual) const {
      HorizontalMotion<T> windBefore =
        BasicTrueWindEstimator::computeTrueWind<T>(x, _before);
      HorizontalMotion<T> windAfter =
        BasicTrueWindEstimator::computeTrueWind<T>(x, _after);
      residual[0] = T(_weight) * (windBefore[0] - windAfter[0]).knots();
      residual[1] = T(_weight) * (windBefore[1] - windAfter[1]).knots();
      return true;
    }

    void printCost(const double* params) {
      HorizontalMotion<double> windBefore =
        BasicTrueWindEstimator::computeTrueWind<double>(params, _before);
      HorizontalMotion<double> windAfter =
        BasicTrueWindEstimator::computeTrueWind<double>(params, _after);
      std::cout << "Wind: " << showWind(windBefore) << " and "
        << showWind(windAfter)
        << " at " << _before[_before.size() - 1].time().toString()
        << " and " << _after[_after.size() - 1].time().toString()
        << " w=" << _weight << "\n";
    }

  private:
    Array<Nav> _before;
    Array<Nav> _after;
    double _weight;
};

void Calibrator::addTack(int pos, double weight) {
  const int length = 5;
  const int delta = 20;
  if ((pos < length + delta) || (pos > _allnavs.size() - length - delta)) {
    LOG(WARNING) << "Ignoring maneuver too close to beginning or end of recording";
    return;
  }

  Array<Nav> before = _allnavs.slice(pos - length - delta, pos - delta);
  Array<Nav> after = _allnavs.slice(pos + delta, pos + delta + length);

  Duration<double> deltaTime = after.first().time() - before.last().time();
  if (deltaTime > Duration<>::minutes(3)) {
    LOG(WARNING) << "Ignoring maneuver with a long time gap.";
    return;
  }

  TackCost *cost = new TackCost(before, after, weight);
  _maneuvers.push_back(cost);
  CostFunction* cost_function =
      new AutoDiffCostFunction<
          TackCost,
          2, //residuals
          BasicTrueWindEstimator::NUM_PARAMS // unknowns
        >(cost);
  _problem.AddResidualBlock(cost_function, new ceres::CauchyLoss(1), _calibrationValues);
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
        LOG(INFO) << "Tack: " << before->right() << " - "
          << after->left() << ": "
          << description(before->child(before->childCount() - 1))
          << "(" << before->child(before->childCount() - 1)->count() << ")"
          << " -> "
          << description(after->child(0))
          << "(" << after->child(0)->count() << ")";
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
  clear();

  // Load data.
  _allnavs = scanNmeaFolder(dataPath, boatId);
  if (_allnavs.size() == 0) {
    return false;
  }

  _tree = _grammar.parse(_allnavs);

  addAllTack(_tree);
  addBuoyTurn(_tree);

  print();

  // Run the solver!
  Solver::Options options;
  options.minimizer_progress_to_stdout = true;
  Solver::Summary summary;
  Solve(options, &_problem, &summary);
  std::cout << summary.BriefReport() << "\n";

  LOG(INFO) << "AWA_OFFSET: " << _calibrationValues[BasicTrueWindEstimator::PARAM_AWA_OFFSET];
  LOG(INFO) << "AWS_BIAS: " << _calibrationValues[BasicTrueWindEstimator::PARAM_AWS_BIAS];

  print();

  return true;
}

void Calibrator::print() {
  for (auto maneuver : _maneuvers) {
    maneuver->printCost(_calibrationValues);
  }
}

void Calibrator::clear() {
  _allnavs.clear();
  _tree.reset();

  BasicTrueWindEstimator::initializeParameters(_calibrationValues);

  _maneuvers.clear();
}

}  // namespace sail

