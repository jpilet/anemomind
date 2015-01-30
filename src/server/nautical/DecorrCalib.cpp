/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <ceres/ceres.h>
#include <server/common/ToDouble.h>
#include "DecorrCalib.h"
#include <server/common/Span.h>
#include <server/common/ScopedLog.h>

namespace sail {

// http://ceres-solver.org/nnls_modeling.html#DynamicAutoDiffCostFunction
namespace {

  template <typename T>
  Array<CalibratedNav<T> > correctSamples(const Corrector<T> &corr, FilteredNavData data) {
    return Spani(0, data.size()).map<CalibratedNav<T> >([&](int index) {
      return corr.correct(data.makeIndexedInstrumentAbstraction(index));
    });
  }


  class Objf {
   public:
    Objf(DecorrCalib *decorr,
          FilteredNavData data);

    template<typename T>
    bool operator()(T const* const* parameters, T* residuals) const {
      const Corrector<T> *corr = (Corrector<T> *)parameters[0];
      return eval(corr, residuals);
    }

    // Correlations between wind and current 2d vectors
    // (Wx, Cx), (Wy, Cy), (Wy, Cx), (Wx, Cy)
    static constexpr int termsPerSample = 4;

    int outDims() const {
      return termsPerSample*_windowCount;
    }

    int inDims() const {
      return Corrector<double>::paramCount();
    }
   private:
    FilteredNavData _data;
    DecorrCalib *_decorr;
    int _windowCount, _absoluteOverlap;

    template <typename T>
    bool eval(const Corrector<T> *corr, T *residuals) const {
      Array<CalibratedNav<T> > corrected = correctSamples(*corr, _data);
    }
  };

  Objf::Objf(DecorrCalib *decorr,
      FilteredNavData data) :
      _data(data),
      _decorr(decorr),
      _absoluteOverlap(decorr->absoluteWindowOverlap()) {

    _windowCount = ((data.size() - _decorr->windowSize())/_absoluteOverlap) + 1;

  }
}



DecorrCalib::Results DecorrCalib::calibrate(FilteredNavData data) {
  ENTER_FUNCTION_SCOPE;
  ceres::Problem problem;

  auto objf = new Objf(this, data);
  auto cost = new ceres::DynamicAutoDiffCostFunction<Objf>(objf);
  cost->SetNumResiduals(objf->outDims());

  Corrector<double> corr;
  problem.AddResidualBlock(cost, NULL, (double *)(&corr));
  ceres::Solver::Options options;
  options.minimizer_progress_to_stdout = true;
  ceres::Solver::Summary summary;
  Solve(options, &problem, &summary);
  SCOPEDMESSAGE(INFO, "Done optimizing.");
  return Results(corr, data);
}

} /* namespace mmm */
