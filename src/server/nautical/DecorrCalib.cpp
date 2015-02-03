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
#include <iostream>
#include <server/common/ArrayIO.h>
#include <server/common/string.h>

namespace sail {

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



// http://ceres-solver.org/nnls_modeling.html#DynamicAutoDiffCostFunction
namespace {

  template <typename T>
  bool messedUp(T x) {
    //return !std::isfinite(x);
    return false;
  }

  template <>
  bool messedUp<ceres::Jet<double, 4> >(ceres::Jet<double, 4> x) {
    if (!std::isfinite(x.a)) {
      //return true;
    }
    for (int i = 0; i < 4; i++) {
      if (!std::isfinite(x.v(i, 0))) {
        std::cout << "x = " << x << std::endl;
        return true;
      }
    }
    return false;
  }

  #define CHECKVALUE(x) assert(!std::isnan(ToDouble(x))); assert(!messedUp(x))

  template <typename T>
  GenericLineKM<T> toLineSub(QuadForm<2, 1, T> qf, int deg) {
    if (deg == 0) {
      // Mean value (polynom of degree 0)
      return GenericLineKM<T>::constant(qf.lineFitY());
    } else {
      assert(deg == 1); // only deg=0 and deg=1 currently supported.
      return qf.makeLine();
    }
  }

  template <typename T>
  GenericLineKM<T> toLine(QuadForm<2, 1, T> qf, int deg) {
    auto line = toLineSub(qf, deg);
    if (messedUp(line.getK()) || messedUp(line.getM())) {
      for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
          std::cout << qf.pElement(i, j) << " ";
        }
        std::cout << "   " << qf.qElement(i, 0) << std::endl;
      }
    }
    CHECKVALUE(line.getK());
    CHECKVALUE(line.getM());
    return line;
  }




  template <typename T>
  CalibratedNav<T> checkCNav(CalibratedNav<T> x) {
    CHECKVALUE(x.calibAwa().degrees());
    CHECKVALUE(x.calibAws().knots());
    CHECKVALUE(x.driftAngle().degrees());
    CHECKVALUE(x.trueCurrent()[0].knots());
    CHECKVALUE(x.trueCurrent()[1].knots());
    CHECKVALUE(x.trueWind()[0].knots());
    CHECKVALUE(x.trueWind()[1].knots());
    return x;
  }

  template <typename T>
  Array<CalibratedNav<T> > correctSamples(const Corrector<T> &corr,
      FilteredNavData data) {
    CHECKVALUE(corr.aws.k);
    CHECKVALUE(corr.aws.m);
    CHECKVALUE(corr.aws.c);
    CHECKVALUE(corr.aws.alpha);
    auto cal = corr.aws.make();
    CHECKVALUE(cal._k);
    CHECKVALUE(cal._m);
    CHECKVALUE(cal._c);
    CHECKVALUE(cal._alpha);

    return Spani(0, data.size()).map<CalibratedNav<T> >([&](int index) {
      return checkCNav(corr.correct(data.makeIndexedInstrumentAbstraction(index)));
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

  template <typename T>
  class Signal1d {
   public:
    Signal1d() : _polyDeg(-1), _offset(0) {}
    Signal1d(bool wind,
        int index,
        Array<CalibratedNav<T> > navs, int polyDeg) : _polyDeg(polyDeg),
     _offset(0) {
      const T init(0.0);
      std::function<HorizontalMotion<T>(CalibratedNav<T>)> extractor = (wind? &(getTrueWind<T>) : &(getTrueCurrent<T>));
      _qf = extractQF(navs, extractor, index);
      _itg = Integral1d<QuadForm<2, 1, T> >(_qf, init);
      updateFit();
    }

    int size() const {
      return _qf.size();
    }

    T operator[] (int index) const {
      return _qf[index].lineFitY() - _fit(T(index + _offset));
    }


    Signal1d(Array<QuadForm<2, 1, T> > qf,
        Integral1d<QuadForm<2, 1, T> > itg,
        int polyDeg, int offset) : _qf(qf),
        _itg(itg), _polyDeg(polyDeg), _offset(offset) {
      updateFit();
    }

    Signal1d<T> slice(int from, int to) const {
      return Signal1d<T>(_qf.slice(from, to), _itg.slice(from, to),
          _polyDeg, _offset + from);
    }

    GenericLineKM<T> line() const {
      return _fit;
    }
   private:
    void updateFit() {
      _fit = toLine(_itg.integral(), _polyDeg);
    }

    Array<QuadForm<2, 1, T> > _qf;
    Integral1d<QuadForm<2, 1, T> > _itg;
    int _polyDeg, _offset;
    GenericLineKM<T> _fit;
  };

  template <typename T>
  class Signal2d {
   public:
    Signal2d(bool wind, Array<CalibratedNav<T> > navs, int polyDeg) {
      data[0] = Signal1d<T>(wind, 0, navs, polyDeg);
      data[1] = Signal1d<T>(wind, 1, navs, polyDeg);
    }
    Signal2d(Signal1d<T> a, Signal1d<T> b) {
      data[0] = a;
      data[1] = b;
    }

    Signal2d<T> slice(int from, int to) const {
      return Signal2d<T>(data[0].slice(from, to), data[1].slice(from, to));
    }

    void eval(int index, T out[2]) const {
      out[0] = data[0][index];
      out[1] = data[1][index];
    }

    int size() const {
      return data[0].size();
    }

    Signal1d<T> data[2];
  };

  template <typename T>
  class Flows {
   public:
    Flows(Array<CalibratedNav<T> > navs, int polyDeg) :
      wind(Signal2d<T>(true, navs, polyDeg)),
      current(Signal2d<T>(false, navs, polyDeg)) {}
    Flows(Signal2d<T> w, Signal2d<T> c) : wind(w), current(c) {}

    Flows<T> slice(int from, int to) const {
      return Flows<T>(wind.slice(from, to), current.slice(from, to));
    }

    int size() const {
      return wind.data[0].size(); // same if we choose data[1] or current
    }

    Signal2d<T> wind, current;
  };




  class Objf {
   public:
    Objf(DecorrCalib *decorr,
          FilteredNavData data,
          Array<Corrector<double> > corrupted);

    template<typename T>
    bool operator()(T const* const* parameters, T* residuals) const {
      const Corrector<T> *corr = (Corrector<T> *)parameters[0];
      return eval(corr, residuals);
    }

    // Correlations between wind and current 2d vectors
    // (Wx, Cx), (Wy, Cy), (Wy, Cx), (Wx, Cy)
    static constexpr int termsPerSample = 4;


    int baseCount() const {
      return termsPerSample*_windowCount;
    }

    int perCorruptorCount() const {
      return 2*baseCount();
    }

    int outDims() const {
      if (_corruptors.empty()) {
        return baseCount();
      }
      return _corruptors.size()*perCorruptorCount();
    }

    int inDims() const {
      return Corrector<double>::paramCount();
    }
   private:
    Array<Corrector<double> > _corruptors;
    Array<Array<CalibratedNav<double> > > _corruptedNavs;


    FilteredNavData _data;
    DecorrCalib *_decorr;
    int _windowCount, _windowStep;

    template <typename T, // Should usually be the type that is passed to eval.
              typename P> // Can be a primitive type, but must not.
    void evalPair(Signal2d<T> a, Signal2d<P> b, T *residuals) const {
      for (int i = 0; i < 2; i++) {
        CHECKVALUE(a.data[i].line().getK());
        CHECKVALUE(a.data[i].line().getM());
        CHECKVALUE(b.data[i].line().getK());
        CHECKVALUE(b.data[i].line().getM());
      }
      assert(a.size() == b.size());

      // Initialize
      for (int i = 0; i < termsPerSample; i++) {
        residuals[i] = T(0);
      }

      // Accumulate
      T wvar[2] = {T(0), T(0)};
      T cvar[2] = {T(0), T(0)};
      int count = a.size();
      for (int k = 0; k < count; k++) {
        T w[2];
        P c[2];
        a.eval(k, w);
        b.eval(k, c);

        for (int i = 0; i < termsPerSample; i++) {
          assert(!messedUp(w[i]));
          assert(!messedUp(c[i]));

          residuals[i] += w[i % 2]*c[i / 2];
        }
        if (_decorr->normalized()) {
          for (int i = 0; i < 2; i++) {
            wvar[i] += sqr(w[i]);
            cvar[i] += sqr(c[i]);
          }
        }
      }

      for (int i = 0; i < 2; i++) {
        assert(!messedUp(wvar[i]));
        wvar[i] = sqrt(wvar[i]/T(count) + 1.0e-9);
        assert(!messedUp(wvar[i]));

        assert(!messedUp(cvar[i]));
        cvar[i] = sqrt(cvar[i]/T(count) + 1.0e-9);
        assert(!messedUp(cvar[i]));
      }

      T f(1.0/count);
      for (int i = 0; i < termsPerSample; i++) {
        auto denom = abs(wvar[i % 2]*cvar[i / 2]);
        assert(!messedUp(denom));
        T factor = f*(_decorr->normalized()? T(1.0)/(denom + 1.0e-9) : T(1.0));
        residuals[i] *= factor;
        assert(!std::isnan(ToDouble(residuals[i])));
        assert(!messedUp(residuals[i]));

        //LOG(INFO) << "Residual: " << ToDouble(residuals[i]);
      }
    }

    template <typename T>
    void evalOldWindow(Flows<T> flows, T *residuals) const {
      evalPair(flows.wind, flows.current, residuals);
    }


    template <typename T>
    bool evalOldSub(Flows<T> flows,
      T *residuals) const {
      const T init(0.0);
      for (int i = 0; i < _windowCount; i++) {
        int left = i*_windowStep;
        int right = left + _decorr->windowSize();
        assert(right <= _data.size());
        evalOldWindow(flows.slice(left, right), residuals + i*termsPerSample);
      }
      return true;
    }

    template <typename T>
    bool eval(const Corrector<T> *corr, T *residuals) const {
      Array<CalibratedNav<T> > cnavs = correctSamples(*corr, _data);
      Flows<T> flows(cnavs, _decorr->polyDeg());
      return evalOldSub(flows, residuals);
    }
  };

  Objf::Objf(DecorrCalib *decorr,
      FilteredNavData data, Array<Corrector<double> > corrupted) :
      _data(data),
      _decorr(decorr),
      _windowStep(decorr->windowStep()),
      _corruptors(corrupted) {
    _corruptedNavs = corrupted.map<Array<CalibratedNav<double> > >([&](Corrector<double> x) {
      return correctSamples(x, data);
    });
    _windowCount = ((data.size() - _decorr->windowSize())/_windowStep) + 1;
    assert(_decorr->windowSize() + (_windowCount - 1)*_windowStep <= data.size());
  }
}



DecorrCalib::Results DecorrCalib::calibrate(FilteredNavData data, Array<Corrector<double> > corrupted) {
  ENTER_FUNCTION_SCOPE;
  ceres::Problem problem;

  auto objf = new Objf(this, data, corrupted);
  auto cost = new ceres::DynamicAutoDiffCostFunction<Objf>(objf);
  cost->AddParameterBlock(Corrector<double>::paramCount());
  cost->SetNumResiduals(objf->outDims());

  Corrector<double> corr;
  problem.AddResidualBlock(cost, NULL, (double *)(&corr));
  ceres::Solver::Options options;
  options.minimizer_progress_to_stdout = true;
  options.max_num_iterations = 60;
  ceres::Solver::Summary summary;
  Solve(options, &problem, &summary);
  SCOPEDMESSAGE(INFO, "Done optimizing.");
  return Results(corr, data);
}

} /* namespace mmm */
