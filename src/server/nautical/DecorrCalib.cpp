/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <ceres/ceres.h>
#include <server/common/ToDouble.h>
#include "DecorrCalib.h"
#include <server/common/Span.h>
#include <server/common/ScopedLog.h>
#include <server/math/QuadForm.h>
#include <server/math/Integral1d.h>

namespace sail {

// http://ceres-solver.org/nnls_modeling.html#DynamicAutoDiffCostFunction
namespace {

  template <typename T>
  Array<CalibratedNav<T> > correctSamples(const Corrector<T> &corr, FilteredNavData data) {
    return Spani(0, data.size()).map<CalibratedNav<T> >([&](int index) {
      return corr.correct(data.makeIndexedInstrumentAbstraction(index));
    });
  }

  template <typename T>
  Array<QuadForm<2, 1, T> > extractQF(Array<CalibratedNav<T> > cnavs,
    std::function<HorizontalMotion<T>(CalibratedNav<T>)> extractor, int index) {
    int count = cnavs.size();
    Array<QuadForm<2, 1, T> > dst(count);
    for (int i = 0; i < count; i++) {
      dst[i] = QuadForm<2, 1, T>::fitLine(T(i), extractor(cnavs[i])[index].knots());
    }
    return dst;
  }

  template <typename T>
  HorizontalMotion<T> getTrueWind(const CalibratedNav<T> &x) {
    return x.trueWind();
  }

  template <typename T>
  HorizontalMotion<T> getTrueCurrent(const CalibratedNav<T> &x) {
    return x.trueCurrent();
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
    bool evalSub(Array<QuadForm<2, 1, T> > W[2], Array<QuadForm<2, 1, T> > C[2]) const {
      const T init(0.0);
      Integral1d<QuadForm<2, 1, T> > Wi[2] = {Integral1d<QuadForm<2, 1, T> >(W[0], init),
                                              Integral1d<QuadForm<2, 1, T> >(W[1], init)};
      Integral1d<QuadForm<2, 1, T> > Ci[2] = {Integral1d<QuadForm<2, 1, T> >(C[0], init),
                                              Integral1d<QuadForm<2, 1, T> >(C[1], init)};
    }

    template <typename T>
    bool eval(const Corrector<T> *corr, T *residuals) const {
      Array<CalibratedNav<T> > cnavs = correctSamples(*corr, _data);

      Array<QuadForm<2, 1, T> > W[2] = {extractQF<T>(cnavs, &(getTrueWind<T>), 0),
                                        extractQF<T>(cnavs, &(getTrueWind<T>), 1)};
      Array<QuadForm<2, 1, T> > C[2] = {extractQF<T>(cnavs, &(getTrueCurrent<T>), 0),
                                        extractQF<T>(cnavs, &(getTrueCurrent<T>), 1)};
      return evalSub(W, C);
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
