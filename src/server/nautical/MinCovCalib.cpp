/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <iostream>
#include <server/nautical/MinCovCalib.h>
#include <ceres/ceres.h>
#include <server/common/Span.h>
#include <device/Arduino/libraries/CalibratedNav/CalibratedNav.h>
#include <server/common/ScopedLog.h>
#include <server/common/string.h>
#include <iostream>

namespace sail {
namespace MinCovCalib {


template <typename T>
Array<CalibratedNav<T> > correct(Corrector<T> corrector, Array<Nav> navs) {
  return navs.map<CalibratedNav<T> >([&](const Nav &x) {
    auto cnav = corrector.correct(x);
    if (cnav.hasNan()) {
      std::cout << EXPR_AND_VAL_AS_STRING(x) << std::endl;
      std::cout << EXPR_AND_VAL_AS_STRING(corrector.awa) << std::endl;
      std::cout << EXPR_AND_VAL_AS_STRING(corrector.toArray()) << std::endl;
      std::cout << EXPR_AND_VAL_AS_STRING(corrector.awa.value) << std::endl;
      std::cout << EXPR_AND_VAL_AS_STRING(corrector) << std::endl;
    }
    assert(!cnav.hasNan());
    return cnav;
  });
}

template <typename T>
Array<T> getOrientationsDegs(Array<CalibratedNav<T> > cnavs) {
  int n = cnavs.size();
  Array<T> dst(n);
  for (int i = 0; i < n; i++) {
    dst[i] = cnavs[i].boatOrientation().degrees();
  }
  return dst;
}

template <typename T>
Array<T> getSpeedsKnots(Array<CalibratedNav<T> > cnavs, bool wind, int indexXY) {
  std::function<T(CalibratedNav<T>)> getSpeedKnots = [&](const CalibratedNav<T> &nav) {
    auto m = (wind? nav.trueWind() : nav.trueCurrent());
    return m[indexXY].knots();
  };
  int n = cnavs.size();
  Array<T> dst(n);
  for (int i = 0; i < n; i++) {
    dst[i] = getSpeedKnots(cnavs[i]);
  }
  return dst;
}

Arrayd getTimes(Array<Nav> navs) {
  auto offset = navs.first().time();
  return navs.map<double>([&](const Nav &x) {
    return (x.time() - offset).seconds();
  });
}

class Objf {
 public:
  Objf(Array<Nav> navs, Settings s) : _navs(navs), _settings(s),
    _residualCountPerPair(s.covarianceSettings.calcResidualCount(navs.size())),
    _times(getTimes(navs)) {}

  int outDims() const {
    return pairCount()*_residualCountPerPair;
  }

  int pairCount() const {
    return 4; // maghdg with windXY and currentXY
  }

  template<typename T>
  bool operator()(T const* const* parameters, T* residuals) const {
    const Corrector<T> *corr = (Corrector<T> *)parameters[0];
    return eval(*corr, residuals);
  }
 private:
  int _residualCountPerPair;
  Settings _settings;
  Array<Nav> _navs;
  Arrayd _times;
  Spani getSpan(int pairIndex) const {
    int offset = pairIndex*_residualCountPerPair;
    return Spani(offset, offset + _residualCountPerPair);
  }

  template <typename T>
  bool eval(const Corrector<T> &corr, T *residualsPtr) const {
    using namespace sail::SignalCovariance;
    const auto &cs = _settings.covarianceSettings;
    Array<T> residuals(outDims(), residualsPtr);
    residuals.setTo(T(0));
    auto cnavs = correct(corr, _navs);

    SignalData<T> orientations(_times, getOrientationsDegs(cnavs), cs);
    assert(pairCount() == 4);
    SignalData<T> quants[4] = {
        SignalData<T>(_times, getSpeedsKnots(cnavs, true, 0), cs),
        SignalData<T>(_times, getSpeedsKnots(cnavs, true, 1), cs),
        SignalData<T>(_times, getSpeedsKnots(cnavs, false, 0), cs),
        SignalData<T>(_times, getSpeedsKnots(cnavs, false, 1), cs)
    };


    T weights[2] = {T(1), T(1)};
    if (_settings.balanced) {
      T variances[2] = {
          quants[0].variance() + quants[1].variance(),
          quants[2].variance() + quants[3].variance()
       };
      for (int i = 0; i < 2; i++) {
        weights[i] = cs.calcWeight(orientations.variance(), variances[i]);
      }
      stringstream ss;
      ss << "Wind weight: " << weights[0] << "\n";
      ss << "Current weight: " << weights[1] << "\n";
    }
    T weightSum = weights[0] + weights[1];

    for (int i = 0; i < 4; i++) {
      auto span = getSpan(i);
      Array<T> dst = residuals.slice(span.minv(), span.maxv());
      auto data = quants[i];
      auto w = weights[i/2]/weightSum;
      evaluateResiduals(w, orientations, data, cs, &dst);
    }
    return true;
  }
};



Corrector<double> optimize(Array<Nav> navs, Settings s) {
  ENTER_FUNCTION_SCOPE;
  Corrector<double> corr;
  ceres::Problem problem;
  auto objf = new Objf(navs, s);
  auto cost = new ceres::DynamicAutoDiffCostFunction<Objf>(objf);
  cost->AddParameterBlock(Corrector<double>::paramCount());
  cost->SetNumResiduals(objf->outDims());
  SCOPEDMESSAGE(INFO, stringFormat("Number of samples: %d", navs.size()));
  SCOPEDMESSAGE(INFO, stringFormat("Number of residuals: %d", objf->outDims()));
  problem.AddResidualBlock(cost, NULL, (double *)(&corr));
  ceres::Solver::Options options;
  options.minimizer_progress_to_stdout = true;
  options.max_num_iterations = 60;
  ceres::Solver::Summary summary;
  SCOPEDMESSAGE(INFO, "Optimizing...");
  Solve(options, &problem, &summary);
  SCOPEDMESSAGE(INFO, "Done optimization.");
  return corr;
}

}
}
