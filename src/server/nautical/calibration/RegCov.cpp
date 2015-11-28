/*
 *  Created on: 2015
 *      Author: Jonas Östlund <jonas@anemomind.com>
 */

#include "RegCov.h"
#include <server/plot/extra.h>

namespace sail {
namespace RegCov {

namespace {
  class LinearFlow {
   public:
    LinearFlow(Eigen::MatrixXd A, Eigen::VectorXd B) : _A(A), _B(B) {}

    template <typename T>
    Array<T> operator() (const T *parameters) const {
      Array<T> dst(_A.rows());
      // Would like to do this with Eigen, but
      // not easy to get it right with T = ceres::Jet
      for (int i = 0; i < _A.rows(); i++) {
        T sum = T(_B(i));
        for (int j = 0; j < _A.cols(); j++) {
          sum += _A(i, j)*parameters[j];
        }
        dst[i] = sum;
      }
      return dst;
    }
   private:
    Eigen::MatrixXd _A;
    Eigen::VectorXd _B;
  };
}

int getMaxIndex(Array<Arrayi> splits) {
  auto max = [](int a, int b) {return std::max(a, b);};
  return map(splits, [=](Arrayi split) {
    return reduce(split, max);
  }).reduce(max);
}

int getDataCount(int dim) {
  int n = dim/2;
  CHECK(n*2 == dim);
  return n;
}

int computeDifCount(int dataSize, int step) {
  return std::max(0, dataSize - 2*step);
}

GnuplotExtra::Settings makePtSettings(std::string c) {
  GnuplotExtra::Settings s;
  s.color = c;
  s.pointType = 0;
  s.pointSize = 1;
  return s;
}

void Summary::plot() const {
  Arrayd allGpsDifs = computeRegDifs(gpsSpeed, settings.step);
  Arrayd allInitialFlowDifs = computeRegDifs(initialFlow, settings.step);
  Arrayd allFinalFlowDifs = computeRegDifs(initialFlow, settings.step);

  GnuplotExtra::Settings initialSettings = makePtSettings("red");
  GnuplotExtra::Settings finalSettings = makePtSettings("green");

  for (auto split: splits) {
    Arrayd gpsDifs = subsetByIndex(allGpsDifs, split).toArray();
    Arrayd initialFlowDifs = subsetByIndex(allInitialFlowDifs, split).toArray();
    Arrayd finalFlowDifs = subsetByIndex(allFinalFlowDifs, split).toArray();

    GnuplotExtra plot;
    plot.defineStyle(1, initialSettings);
    plot.set_current_line_style(1);
    plot.setEqualAxes();
    plot.plot_xy(gpsDifs, initialFlowDifs);
    plot.set_style("lines");
    plot.defineStyle(2, finalSettings);
    plot.set_current_line_style(2);
    plot.plot_xy(gpsDifs, finalFlowDifs);
    plot.show();
  }
}

Summary optimizeLinear(Eigen::MatrixXd A,
                      Eigen::VectorXd B,
                      Array<Arrayi> splits, Arrayd initialParameters,
                      Settings settings) {
  CHECK(initialParameters.size() == A.cols());
  Arrayd gpsData(B.size(), B.data());
  return optimize(LinearFlow(A, B), gpsData, splits, initialParameters, settings);
}


}
}
