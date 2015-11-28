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
  s.pointType = 7;
  s.pointSize = 1;
  return s;
}


void plotInitialAndFinalDifs(Arrayd gpsDifs, Arrayd initialFlowDifs, Arrayd finalFlowDifs) {
  GnuplotExtra::Settings initialSettings = makePtSettings("red");
  GnuplotExtra::Settings finalSettings = makePtSettings("green");
  GnuplotExtra plot;
  plot.set_style("points");
  plot.defineStyle(1, initialSettings);
  plot.set_current_line_style(1);
  plot.setEqualAxes();
  plot.plot_xy(initialFlowDifs, gpsDifs);
  plot.defineStyle(2, finalSettings);
  plot.set_current_line_style(2);
  plot.plot_xy(finalFlowDifs, gpsDifs);
  //plot.setEqualAxes();
  plot.show();
}

void Summary::plot() const {
  std::cout << EXPR_AND_VAL_AS_STRING(gpsSpeed) << std::endl;
  Arrayd allGpsDifs = computeRegDifs(gpsSpeed, settings.step);
  Arrayd allInitialFlowDifs = computeRegDifs(initialFlow, settings.step);
  Arrayd allFinalFlowDifs = computeRegDifs(finalFlow, settings.step);

  std::cout << EXPR_AND_VAL_AS_STRING(allGpsDifs.sliceTo(30)) << std::endl;
  std::cout << EXPR_AND_VAL_AS_STRING(allInitialFlowDifs.sliceTo(30)) << std::endl;
  std::cout << EXPR_AND_VAL_AS_STRING(allFinalFlowDifs.sliceTo(30)) << std::endl;

  plotInitialAndFinalDifs(allGpsDifs, allInitialFlowDifs, allFinalFlowDifs);

  for (auto split: splits) {
    Arrayd gpsDifs = subsetByIndex(allGpsDifs, split).toArray();
    Arrayd initialFlowDifs = subsetByIndex(allInitialFlowDifs, split).toArray();
    Arrayd finalFlowDifs = subsetByIndex(allFinalFlowDifs, split).toArray();

    plotInitialAndFinalDifs(gpsDifs, initialFlowDifs, finalFlowDifs);
  }
}

Summary optimizeLinear(Eigen::MatrixXd A,
                      Eigen::VectorXd B,
                      Array<Arrayi> splits, Arrayd initialParameters,
                      Settings settings) {
  CHECK(initialParameters.size() == A.cols());
  Arrayd gpsData = Arrayd(B.size(), B.data()).dup();
  return optimize(LinearFlow(A, B), gpsData, splits, initialParameters, settings);
}


}
}
