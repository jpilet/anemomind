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

  adouble roundedAbs(adouble x0, double width) {
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

  double makePositive(double x) {
    if (x < 0) {
      return 0.001;
    }
    return x;
  }

  class Objf : public Function {
   public:
    static constexpr int eqsPerComparison = 4;

    Objf(FilteredNavData data, CorrectorSet<adouble>::Ptr corr, Arrayd times);

    int inDims() {
      return _corr->paramCount() + 1;
    }

    int outDims() {
      return eqsPerComparison*_times.size();
    }

    void eval(double *Xin, double *Fout, double *Jout);
   private:
    FilteredNavData _data;
    CorrectorSet<adouble>::Ptr _corr;
    Arrayd _times, _weights;
    LineKM _sampling;

    adouble balance(adouble *parameters) {
      return parameters[_corr->paramCount()];
    }
    CalibratedValues<adouble> compute(int time, adouble *parameters);
    CalibratedValues<adouble> computeTest(int index, adouble *parameters);

    void evalDif(double w, CalibratedValues<adouble> a,
        CalibratedValues<adouble> b, adouble balance, adouble *dst);
  };

  CalibratedValues<adouble> Objf::compute(int index, adouble *parameters) {

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

  CalibratedValues<adouble> Objf::computeTest(int index, adouble *parameters) {
    Velocity<adouble> v0 = Velocity<adouble>::knots(1);
    Angle<adouble> a0 = Angle<adouble>::degrees(0);
    HorizontalMotion<adouble> gpsMotion =
        HorizontalMotion<adouble>::polar(v0, a0);

    return CalibratedValues<adouble>(*_corr,
                  parameters,
                  gpsMotion,
                  a0, v0, a0, v0);
  }

  Objf::Objf(FilteredNavData data, CorrectorSet<adouble>::Ptr corr,
    Arrayd times) :
      _data(data), _corr(corr),
      _times(times),
      _weights(data.magHdg().interpolateLinearDerivative(times).map<double>([&](Angle<double> x) {return x.degrees();})),
      _sampling(data.sampling()) {}

  void Objf::eval(double *Xin, double *Fout, double *Jout) {
    ENTERSCOPE("Evaluating calibration objf");
    assert(_times.size() == _weights.size());


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
    for (int i = 0; i < _times.size(); i++) {
      if (Jout != nullptr) {
        trace_on(tape);
      }

      adolcInput(inDims(), adX.getData(), Xin);
      if (i % 10000 == 0) {
        SCOPEDMESSAGE(INFO, stringFormat("Iteration %d/%d", i, _times.size()));
      }
      int index = int(floor(_sampling.inv(_times[i])));
      CalibratedValues<adouble> from = compute(index, adX.ptr());
      CalibratedValues<adouble> to = compute(index + 1, adX.ptr());
      evalDif(_weights[i], from, to, balance(adX.ptr()), adF.getData());
      adolcOutput(4, adF.getData(), Fout + i*eqsPerComparison);

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

    double width = 0.1; // knots per second

    dst[0] = roundedAbs(balance*w*(a.trueWind[0].knots() - b.trueWind[0].knots()), width);
    dst[1] = roundedAbs(balance*w*(a.trueWind[1].knots() - b.trueWind[1].knots()), width);
    dst[2] = roundedAbs((1.0 - balance)*w*(a.trueCurrent[0].knots() - b.trueCurrent[0].knots()), width);
    dst[3] = roundedAbs((1.0 - balance)*w*(a.trueCurrent[1].knots() - b.trueCurrent[1].knots()), width);
  }

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
}

CalibratedNavData::CalibratedNavData(FilteredNavData filteredData,
      Arrayd times, CorrectorSet<adouble>::Ptr correctorSet,
      LevmarSettings settings, Arrayd initialization) : _filteredRawData(filteredData) {
  ENTERSCOPE("CalibratedNavData");
  _correctorSet = (bool(correctorSet)? correctorSet : CorrectorSet<adouble>::Ptr(new DefaultCorrectorSet<adouble>()));
  if (times.empty()) {
    times = filteredData.makeCenteredX();
  }
  Objf objf(filteredData, _correctorSet, times);
  SCOPEDMESSAGE(INFO, stringFormat("Objf dimensions: %d", objf.outDims()));
  if (initialization.empty()) {
    Arrayad initParams(_correctorSet->paramCount());
    _correctorSet->initialize(initParams.ptr());
    initialization = initParams.map<double>([&](adouble x) {return x.getValue();});
  }
  initialization = addBalanceParam(initialization);
  _initialCalibrationParameters = initialization.dup();
  assert(initialization.size() == objf.inDims());
  LevmarState state(initialization);
  state.minimize(settings, objf);
  _optimalCalibrationParameters = state.getXArray(true);
  _value = objf.calcSquaredNorm(_optimalCalibrationParameters.ptr());
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
  assert(initializations.hasData());
  CalibratedNavData best(fdata, times, correctorSet, settings,
      initializations.first());
  for (int i = 1; i < initializations.size(); i++) {
    CalibratedNavData candidate(fdata, times, correctorSet, settings,
        initializations[i]);
    if (candidate < best) {
      best = candidate;
    }
  }
  return best;
}

}
