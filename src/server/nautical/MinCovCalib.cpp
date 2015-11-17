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
#include <random>

namespace sail {
namespace MinCovCalib {



namespace {
  AngleCorrector<double> makeRandomAngleCorrector(std::default_random_engine &e) {
    constexpr double maxDeg = 45;
    std::uniform_real_distribution<double> distrib(-maxDeg, maxDeg);
    return AngleCorrector<double>(Angle<double>::degrees(distrib(e)));
  }

  SpeedCorrector<double> makeRandomSpeedCorrector(std::default_random_engine &e) {
      std::uniform_real_distribution<double> mDistrib(0.01, 1);
      std::uniform_real_distribution<double> kDistrib(-1, 1);
      return SpeedCorrector<double>(mDistrib(e), kDistrib(e),
          SpeedCalib<double>::initCParam(), SpeedCalib<double>::initAlphaParam());
  }

  DriftAngleCorrector<double> makeRandomDriftAngleCorrector(std::default_random_engine &e) {
    std::uniform_real_distribution<double> amp(0, Angle<double>::degrees(10).radians());
    std::uniform_real_distribution<double> coef(0, 1);
    return DriftAngleCorrector<double>(amp(e), coef(e));
  }

}


Corrector<double> makeRandomCorrector(std::default_random_engine &e) {
  Corrector<double> c;
  c.awa = makeRandomAngleCorrector(e);
  c.magHdg = makeRandomAngleCorrector(e);
  c.aws = makeRandomSpeedCorrector(e);
  c.watSpeed = makeRandomSpeedCorrector(e);
  c.driftAngle = makeRandomDriftAngleCorrector(e);
  return c;
}

Array<Corrector<double> > makeRandomCorrectors(int count, std::default_random_engine &e) {
  Array<Corrector<double> > corrs(count);
  std::cout << "Array<Arrayd> corrdata(" << count << ");" << std::endl;
  for (int i = 0; i <  count; i++) {
    corrs[i] = makeRandomCorrector(e);
    Arrayd d = corrs[i].toArray();
    std::cout << stringFormat("corrdata[%d] = Arrayd", i);
    outputArrayLiteral(d, &(std::cout));
  }
  return corrs;
}

Array<Corrector<double> > makeCorruptCorrectors() {
  Array<Arrayd> corrdata(30);
  corrdata[0] = Arrayd{-33.1616, -3.72149, 0.682076, -0.562082, 0.01, 0, 0.524222, 0.869386, 0.01, 0, 0.0924501, 0.0345721};
  corrdata[1] = Arrayd{-44.3072, -38.9842, 0.931132, 0.373545, 0.01, 0, 0.65738, 0.0538576, 0.01, 0, 0.133029, 0.701191};
  corrdata[2] = Arrayd{-40.7282, -15.4589, 0.371685, 0.512821, 0.01, 0, 0.755822, 0.965101, 0.01, 0, 0.154411, 0.0726859};
  corrdata[3] = Arrayd{-5.72297, -2.00414, 0.174842, -0.450186, 0.01, 0, 0.0699587, 0.795313, 0.01, 0, 0.0556818, 0.504523};
  corrdata[4] = Arrayd{-0.542098, -36.834, 0.390301, -0.852502, 0.01, 0, 0.469801, 0.827635, 0.01, 0, 0.134426, 0.050084};
  corrdata[5] = Arrayd{-33.7171, 16.961, 0.728158, 0.259087, 0.01, 0, 0.313259, 0.777144, 0.01, 0, 0.147652, 0.513274};
  corrdata[6] = Arrayd{30.736, -7.61448, 0.186544, -0.0641653, 0.01, 0, 0.0427232, 0.14331, 0.01, 0, 0.130602, 0.49848};
  corrdata[7] = Arrayd{35.1664, 30.7836, 0.139123, -0.574497, 0.01, 0, 0.42015, -0.450824, 0.01, 0, 0.0418723, 0.70982};
  corrdata[8] = Arrayd{-16.4214, 13.6853, 0.393848, 0.362692, 0.01, 0, 0.84712, -0.704934, 0.01, 0, 0.0258573, 0.955409};
  corrdata[9] = Arrayd{-8.211, 5.84088, 0.961484, -0.0229709, 0.01, 0, 0.632976, -0.600486, 0.01, 0, 0.140163, 0.651254};
  corrdata[10] = Arrayd{-2.12114, -26.7075, 0.150601, 0.803347, 0.01, 0, 0.886792, -0.179374, 0.01, 0, 0.0637637, 0.162199};
  corrdata[11] = Arrayd{-32.8402, -4.02234, 0.932358, -0.0953997, 0.01, 0, 0.909833, -0.569503, 0.01, 0, 0.088306, 0.86086};
  corrdata[12] = Arrayd{28.5805, -3.39795, 0.82645, 0.265477, 0.01, 0, 0.954871, 0.404413, 0.01, 0, 0.0897858, 0.289316};
  corrdata[13] = Arrayd{-7.73744, 33.8909, 0.718486, 0.459495, 0.01, 0, 0.0289015, 0.413071, 0.01, 0, 0.0113785, 0.524987};
  corrdata[14] = Arrayd{-0.995116, 16.3844, 0.891118, 0.833268, 0.01, 0, 0.989469, -0.72161, 0.01, 0, 0.089825, 0.446023};
  corrdata[15] = Arrayd{-5.4247, 27.5985, 0.162068, -0.576962, 0.01, 0, 0.0108702, 0.232701, 0.01, 0, 0.0729065, 0.727335};
  corrdata[16] = Arrayd{16.2506, 30.2778, 0.0909202, 0.657416, 0.01, 0, 0.221411, 0.259144, 0.01, 0, 0.165378, 0.388823};
  corrdata[17] = Arrayd{-20.7707, -19.4368, 0.289334, 0.56773, 0.01, 0, 0.983403, -0.977368, 0.01, 0, 0.0694892, 0.819726};
  corrdata[18] = Arrayd{-29.0808, -30.8042, 0.110621, -0.485663, 0.01, 0, 0.796822, 0.269435, 0.01, 0, 0.110554, 0.75294};
  corrdata[19] = Arrayd{8.8395, -16.31, 0.530862, -0.765126, 0.01, 0, 0.70596, 0.175978, 0.01, 0, 0.150138, 0.161688};
  corrdata[20] = Arrayd{5.11522, -16.5593, 0.592238, 0.0570965, 0.01, 0, 0.376524, -0.138304, 0.01, 0, 0.0676893, 0.446866};
  corrdata[21] = Arrayd{-37.9563, -22.147, 0.731322, 0.352474, 0.01, 0, 0.940761, 0.889506, 0.01, 0, 0.115428, 0.460434};
  corrdata[22] = Arrayd{9.50757, -31.4645, 0.52758, -0.312365, 0.01, 0, 0.646549, -0.970099, 0.01, 0, 0.169312, 0.917848};
  corrdata[23] = Arrayd{30.9578, -3.98236, 0.70988, -0.955833, 0.01, 0, 0.754193, -0.126723, 0.01, 0, 0.131574, 0.695679};
  corrdata[24] = Arrayd{-44.6171, -37.8075, 0.749212, 0.806602, 0.01, 0, 0.523294, -0.987317, 0.01, 0, 0.0376687, 0.544023};
  corrdata[25] = Arrayd{11.4175, 18.8388, 0.378191, -0.439422, 0.01, 0, 0.112759, -0.379662, 0.01, 0, 0.00964797, 0.77588};
  corrdata[26] = Arrayd{-30.5753, -18.268, 0.524789, -0.0443914, 0.01, 0, 0.835345, -0.699377, 0.01, 0, 0.0578009, 0.577776};
  corrdata[27] = Arrayd{43.8195, -44.8508, 0.494755, -0.941289, 0.01, 0, 0.540904, 0.477917, 0.01, 0, 0.0328603, 0.122507};
  corrdata[28] = Arrayd{10.2713, -33.666, 0.839721, 0.908057, 0.01, 0, 0.430093, -0.481972, 0.01, 0, 0.132753, 0.897914};
  corrdata[29] = Arrayd{-4.33339, 14.8953, 0.551505, -0.580875, 0.01, 0, 0.437589, -0.648051, 0.01, 0, 0.145946, 0.687414};
  return corrdata.map<Corrector<double> >([&](Arrayd x) {
    return *(Corrector<double>::fromArray(x));
  });
}


template <typename T>
Array<CalibratedNav<T> > correct(Corrector<T> corrector, FilteredNavData data) {
  return Spani(0, data.size()).map<CalibratedNav<T> >([&](int index) {
    auto x = data.makeIndexedInstrumentAbstraction(index);
    auto cnav = corrector.correct(x);
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
    auto m = (wind? nav.trueWindOverGround() : nav.trueCurrentOverGround());
    return m[indexXY].knots();
  };
  int n = cnavs.size();
  Array<T> dst(n);
  for (int i = 0; i < n; i++) {
    auto x = getSpeedKnots(cnavs[i]);
    assert(!genericIsNan(x));
    dst[i] = x;
  }
  return dst;
}

Arrayd getTimes(FilteredNavData data) {
  return data.timesSinceOffset().map<double>([&](Duration<double> x) {
    return x.seconds();
  });
}

class ObjfOrientation {
 public:
  ObjfOrientation(FilteredNavData data, Settings s) : _data(data), _settings(s),
    _residualCountPerPair(s.covarianceSettings.calcResidualCount(data.size())),
    _times(getTimes(data)) {}

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
  FilteredNavData _data;
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
    auto cnavs = correct(corr, _data);

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

template <typename Objf>
Corrector<double> optimizeForObjf(FilteredNavData data, Settings s,
    Objf *objf) {
  ENTER_FUNCTION_SCOPE;
  Corrector<double> corr;
  ceres::Problem problem;
  auto cost = new ceres::DynamicAutoDiffCostFunction<Objf>(objf);
  cost->AddParameterBlock(Corrector<double>::paramCount());
  cost->SetNumResiduals(objf->outDims());
  SCOPEDMESSAGE(INFO, stringFormat("Number of samples: %d", data.size()));
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

Corrector<double> optimizeByOrientation(FilteredNavData data, Settings s) {
  return optimizeForObjf(
      data, s,
      new ObjfOrientation(data, s));
}





class ObjfWindVsCurrent {
 public:
  ObjfWindVsCurrent(FilteredNavData data, Settings settings);

  int outDims() const {
    return pairCount()*_residualCountPerPair;
  }

  int pairCount() const {
    return 2*2*2*_corruptData.size();
  }

  template<typename T>
  bool operator()(T const* const* parameters, T* residuals) const {
    const Corrector<T> *corr = (Corrector<T> *)parameters[0];
    return eval(*corr, residuals);
  }
 private:
  Array<Array<CalibratedNav<double> > > _corruptData;
  FilteredNavData _data;
  Settings _settings;
  Arrayd _times;
  int _residualCountPerPair;

  template <typename T>
    bool eval(const Corrector<T> &corr, T *residualsPtr) const {
      using namespace sail::SignalCovariance;
      const auto &cs = _settings.covarianceSettings;
      Array<T> residuals(outDims(), residualsPtr);
      residuals.setTo(T(0));
      auto cnavs = correct(corr, _data);
      assert(cnavs.size() > 0);
      int from = 0;
      for (int order = 0; order < 2; order++) {
        for (int activeDim = 0; activeDim < 2; activeDim++) {
          auto active = SignalData<T>(_times, getSpeedsKnots<T>(cnavs, order == 0, activeDim), cs);
          assert(active.X.size() > 0);
          for (int i = 0; i < _corruptData.size(); i++) {
            for (int passiveDim = 0; passiveDim < 2; passiveDim++) {
              int to = from + _residualCountPerPair;
              auto subResiduals = residuals.slice(from, to);
              auto passive = SignalData<double>(_times,
                  getSpeedsKnots(_corruptData[i], order == 1, passiveDim), cs);
              assert(passive.X.size() > 0);
              evaluateResiduals<T, double>(T(1.0), active, passive, cs, &subResiduals);
              from = to;
            }
          }
        }
      }
      assert(from == outDims());
      return true;
    }
};

ObjfWindVsCurrent::ObjfWindVsCurrent(FilteredNavData data, Settings settings) :
    _settings(settings), _data(data) {
  _corruptData = settings.corruptors.map<Array<CalibratedNav<double> > >([&](Corrector<double> corr) {
    return correct(corr, data);
  });
  _residualCountPerPair = settings.covarianceSettings.calcResidualCount(data.size());
  _times = getTimes(data);
  std::cout << EXPR_AND_VAL_AS_STRING(outDims()) << std::endl;
}

Corrector<double> optimizeWindVsCurrent(FilteredNavData data, Settings s) {
  return optimizeForObjf(data, s, new ObjfWindVsCurrent(data, s));
}

}
}
