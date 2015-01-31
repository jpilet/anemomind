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
    int _windowCount, _windowStep;

    template <typename T>
    void evalWindow(QuadForm<2, 1, T> Wl[2], QuadForm<2, 1, T> Cl[2],
        Array<QuadForm<2, 1, T> > Wslice[2],
        Array<QuadForm<2, 1, T> > Cslice[2], T *residuals) const {

    }

    template <typename T>
    bool evalSub(Array<QuadForm<2, 1, T> > W[2], Array<QuadForm<2, 1, T> > C[2],
      T *residuals) const {
      const T init(0.0);
      Integral1d<QuadForm<2, 1, T> > Wi[2] = {Integral1d<QuadForm<2, 1, T> >(W[0], init),
                                              Integral1d<QuadForm<2, 1, T> >(W[1], init)};
      Integral1d<QuadForm<2, 1, T> > Ci[2] = {Integral1d<QuadForm<2, 1, T> >(C[0], init),
                                              Integral1d<QuadForm<2, 1, T> >(C[1], init)};
      for (int i = 0; i < _windowCount; i++) {
        int left = i*_windowStep;
        int right = left + _decorr->windowSize();
        assert(right <= _data.size());

        QuadForm<2, 1, T> Wl[2] = {Wi[0].integrate(left, right),
                                   Wi[1].integrate(left, right)};
        QuadForm<2, 1, T> Cl[2] = {Ci[0].integrate(left, right),
                                   Ci[1].integrate(left, right)};

        Array<QuadForm<2, 1, T> > Wslice[2] = {W[0].slice(left, right),
                                               W[1].slice(left, right)};
        Array<QuadForm<2, 1, T> > Cslice[2] = {C[0].slice(left, right),
                                               C[1].slice(left, right)};

        evalWindow(Wl, Cl, Wslice, Cslice, residuals + i*termsPerSample);
      }
    }

    template <typename T>
    bool eval(const Corrector<T> *corr, T *residuals) const {
      Array<CalibratedNav<T> > cnavs = correctSamples(*corr, _data);

      Array<QuadForm<2, 1, T> > W[2] = {extractQF<T>(cnavs, &(getTrueWind<T>), 0),
                                        extractQF<T>(cnavs, &(getTrueWind<T>), 1)};
      Array<QuadForm<2, 1, T> > C[2] = {extractQF<T>(cnavs, &(getTrueCurrent<T>), 0),
                                        extractQF<T>(cnavs, &(getTrueCurrent<T>), 1)};
      return evalSub(W, C, residuals);
    }
  };

  Objf::Objf(DecorrCalib *decorr,
      FilteredNavData data) :
      _data(data),
      _decorr(decorr),
      _windowStep(decorr->windowStep()) {

    _windowCount = ((data.size() - _decorr->windowSize())/_windowStep) + 1;
    assert(_decorr->windowSize() + (_windowCount - 1)*_windowStep <= data.size());
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
