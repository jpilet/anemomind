/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/math/ADFunction.h>
#include <server/nautical/CalibratedNavData.h>
#include <server/math/nonlinear/Levmar.h>
#include <server/math/armaadolc.h>
#include <server/common/ScopedLog.h>
#include <server/common/string.h>
#include <adolc/taping.h>
#include <server/common/ProportionateSampler.h>


namespace sail {

namespace {

  /*
   * A cost function that looks like the square root of absolute value,
   * except that it is smooth close to 0, so that we
   * can use it with a smooth optimization algorithm.
   *
   * The width parameter controls how close to 0 it should
   * be smooth.
   *
   * This function will tend to encourage sparse solutions,
   * in contrast to the square function.
   *
   * The sqrt function is applied, so that we can use it
   * with the Levenberg-Marquardt framework.
   */
  adouble sqrtRoundedAbs(adouble x0, double width) {
    adouble x = x0/width;
    double xd = ToDouble(x);
    if (std::abs(xd) < 1.0) {
      constexpr double a = 0.5;
      constexpr double b = 0.5;
      return sqrt(a*x*x + b);
    } else {
      return sqrt(fabs(x));
    }
  }

  adouble applyCostFun(adouble x, double w) {
    return x;
  }

  double makePositive(double x) {
    if (x < 0) {
      return 0.001;
    }
    return x;
  }

  class BaseObjf {
   public:
    static constexpr int eqsPerComparison = 4;

    BaseObjf(FilteredNavData data, CorrectorSet<adouble>::Ptr corr, Arrayd times);

    FilteredNavData _data;
    CorrectorSet<adouble>::Ptr _corr;
    Arrayd _times, _weights;
    LineKM _sampling;

    adouble balance(adouble *parameters) {
      return parameters[_corr->paramCount()];
    }
    CalibratedValues<adouble> compute(int time, adouble *parameters);
  };



  CalibratedValues<adouble> BaseObjf::compute(int index, adouble *parameters) {

    HorizontalMotion<adouble> gpsMotion =
        HorizontalMotion<adouble>::polar(_data.gpsSpeed().get(index).cast<adouble>(),
            _data.gpsBearing().get(index).cast<adouble>());

    return CalibratedValues<adouble>(*_corr,
                  parameters,
                  gpsMotion,
                  Angle<adouble>::degrees(_data.magHdg().get(index).degrees()),
                  Velocity<adouble>::knots(makePositive(_data.watSpeed().get(index).knots())),
                  Angle<adouble>::degrees(_data.awa().get(index).degrees()),
                  Velocity<adouble>::knots(makePositive(_data.aws().get(index).knots())));
  }

  BaseObjf::BaseObjf(FilteredNavData data, CorrectorSet<adouble>::Ptr corr,
    Arrayd times) :
      _data(data), _corr(corr),
      _times(times),
      _weights(data.magHdg().interpolateLinearDerivative(times).map<double>([&](Angle<double> x) {return x.degrees();})),
      _sampling(data.sampling()) {}



  /*
   * Add a parameter that controls the balance
   * between the current and wind fitness.
   */
  Arrayd addBalanceParam(Arrayd calibParams) {
    int count = calibParams.size();
    Arrayd dst(count + 1);
    dst.last() = 0.5;
    calibParams.copyToSafe(dst.sliceBut(1));
    return dst;
  }


  class Objf : public Function {
   public:
    Objf(FilteredNavData data, CorrectorSet<adouble>::Ptr corr, Arrayd times);

    int inDims() {
      return _base._corr->paramCount() + 1;
    }

    int outDims() {
      return BaseObjf::eqsPerComparison*_base._times.size();
    }

   private:
    BaseObjf _base;

    void evalDif(double w, CalibratedValues<adouble> a,
        CalibratedValues<adouble> b, adouble balance, adouble *dst);
    void eval(double *Xin, double *Fout, double *Jout);
  };

  Objf::Objf(FilteredNavData data, CorrectorSet<adouble>::Ptr corr, Arrayd times) :
    _base(data, corr, times) {}


  void Objf::eval(double *Xin, double *Fout, double *Jout) {
    ENTERSCOPE("Evaluating calibration objf");
    assert(_base._times.size() == _base._weights.size());


    Arrayad adX(inDims());
    Arrayad adF(4);

    int rows = outDims();
    int cols = inDims();
    MDArray2d Jmat;
    if (Jout != nullptr) {
      Jmat = MDArray2d(rows, cols, Jout);
    }

    /*
     * Here we differentiate each
     * iteration separately. It turns
     * out that this results in a
     * significant speed-up. Not doing
     * so is painfully slow, for some
     * reason.
     */
    short int tape = 0;
    for (int i = 0; i < _base._times.size(); i++) {
      if (Jout != nullptr) {
        trace_on(tape);
      }

      adolcInput(inDims(), adX.getData(), Xin);
      if (i % 10000 == 0) {
        SCOPEDMESSAGE(INFO, stringFormat("Iteration %d/%d", i, _base._times.size()));
      }
      int index = int(floor(_base._sampling.inv(_base._times[i])));
      CalibratedValues<adouble> from = _base.compute(index, adX.ptr());
      CalibratedValues<adouble> to = _base.compute(index + 1, adX.ptr());
      double weight = sqrt(std::abs(_base._weights[i]));
      evalDif(weight, from, to, _base.balance(adX.ptr()), adF.getData());
      adolcOutput(4, adF.getData(), Fout + i*_base.eqsPerComparison);

      if (Jout != nullptr) {
        trace_off();
        outputJacobianColMajor(tape, Xin, Jmat.getPtrAt(i, 0), rows);
      }
    }
  }

  void Objf::evalDif(double w, CalibratedValues<adouble> a,
      CalibratedValues<adouble> b, adouble balance, adouble *dst) {

    // Skip the balancing.
    balance = 0.5;

    double width = 0.01; // knots per second

    dst[0] = applyCostFun(balance*w*(a.trueWind[0].knots() - b.trueWind[0].knots()), width);
    dst[1] = applyCostFun(balance*w*(a.trueWind[1].knots() - b.trueWind[1].knots()), width);
    dst[2] = applyCostFun((1.0 - balance)*w*(a.trueCurrent[0].knots() - b.trueCurrent[0].knots()), width);
    dst[3] = applyCostFun((1.0 - balance)*w*(a.trueCurrent[1].knots() - b.trueCurrent[1].knots()), width);
  }
}

namespace {

  CorrectorSet<adouble>::Ptr makeDefaultCorr(CorrectorSet<adouble>::Ptr correctorSet) {
    return (bool(correctorSet)? correctorSet : CorrectorSet<adouble>::Ptr(new DefaultCorrectorSet<adouble>()));
  }

}

CalibratedNavData::CalibratedNavData(FilteredNavData filteredData,
      Arrayd times, CorrectorSet<adouble>::Ptr correctorSet,
      LevmarSettings settings, Arrayd initialization) : _filteredRawData(filteredData) {
  ENTERSCOPE("CalibratedNavData");
  _correctorSet = makeDefaultCorr(correctorSet);
  if (times.empty()) {
    times = filteredData.makeCenteredX();
  }
  Objf objf(filteredData, _correctorSet, times);
  SCOPEDMESSAGE(INFO, stringFormat("Objf dimensions: %d", objf.outDims()));
  if (initialization.empty()) {
    initialization = _correctorSet->makeInitialParams(0.0);
  }
  initialization = addBalanceParam(initialization);
  _initialCalibrationParameters = initialization.dup();
  assert(initialization.size() == objf.inDims());
  LevmarState state(initialization);
  state.minimize(settings, objf);
  _optimalCalibrationParameters = state.getXArray(true);

  _initCost = objf.calcSquaredNorm(_initialCalibrationParameters.ptr());
  _cost = objf.calcSquaredNorm(_optimalCalibrationParameters.ptr());
}

Arrayd CalibratedNavData::sampleTimes(FilteredNavData navdata, int count) {
  Arrayd times = navdata.makeCenteredX();
  if (times.size() < count) {
    return times;
  }

  Arrayd maghdg = navdata.magHdg().interpolateLinear(times).map<double>([&](Angle<double> x) {
    return std::abs(x.degrees());
  });

  ProportionateSampler prop(maghdg);
  double marg = 1.0e-6;
  Arrayd selected(count);
  LineKM line(0, count-1, marg, 1.0-marg);
  for (int i = 0; i < count; i++) {
    selected[i] = times[prop.getAndRemove(line(i))];
  }
  return selected;
}

CalibratedNavData CalibratedNavData::bestOfInits(Array<Arrayd> initializations,
    FilteredNavData fdata, Arrayd times,
    CorrectorSet<adouble>::Ptr correctorSet,
             LevmarSettings settings) {
  correctorSet = makeDefaultCorr(correctorSet);
  ENTERSCOPE("Calibration starting from different initializations");
  assert(initializations.hasData());
  CalibratedNavData best;
  for (int i = 0; i < initializations.size(); i++) {
    SCOPEDMESSAGE(INFO, stringFormat("Calibrating from starting point %d/%d",
        i+1, initializations.size()));
    CalibratedNavData candidate(fdata, times, correctorSet, settings,
        initializations[i]);
    SCOPEDMESSAGE(INFO, stringFormat("Cost: %.8g   Initial cost: %.8g (best cost: %.8g)", candidate.cost(),
        candidate.initCost(), best.cost()));
    if (candidate < best) {
      best = candidate;
      SCOPEDMESSAGE(INFO, "  Improvement :-)");
    }
  }
  return best;
}

CalibratedNavData CalibratedNavData::bestOfInits(int initCount,
    FilteredNavData fdata, Arrayd times,
    CorrectorSet<adouble>::Ptr correctorSet,
             LevmarSettings settings) {
  correctorSet = makeDefaultCorr(correctorSet);
  assert(initCount > 0);
  Array<Arrayd> inits(initCount);

  // Gradually increase the randomness...
  LineKM randomness(0, std::max(1, initCount-1), 0.0, 0.2);

  for (int i = 0; i < initCount; i++) {
    inits[i] = correctorSet->makeInitialParams(randomness(i));
  }
  return bestOfInits(inits, fdata, times, correctorSet, settings);
}

}
