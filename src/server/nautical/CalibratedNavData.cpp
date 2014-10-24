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
      return _corr->paramCount();
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

    CalibratedValues<adouble> compute(int time, adouble *parameters);
    CalibratedValues<adouble> computeTest(int index, adouble *parameters);

    void evalDif(double w, CalibratedValues<adouble> a,
        CalibratedValues<adouble> b, adouble *dst);
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
      evalDif(_weights[i], from, to, adF.getData());
      adolcOutput(4, adF.getData(), Fout + i*eqsPerComparison);

      if (Jout != nullptr) {
        trace_off();
        outputJacobianColMajor(tape, Xin, Jmat.getPtrAt(i, 0), rows);
      }
    }
  }

  void Objf::evalDif(double w, CalibratedValues<adouble> a,
      CalibratedValues<adouble> b, adouble *dst) {
    dst[0] = w*(a.trueWind[0].knots() - b.trueWind[0].knots());
    dst[1] = w*(a.trueWind[1].knots() - b.trueWind[1].knots());
    dst[2] = w*(a.trueCurrent[0].knots() - b.trueCurrent[0].knots());
    dst[3] = w*(a.trueCurrent[1].knots() - b.trueCurrent[1].knots());
  }


}

CalibratedNavData::CalibratedNavData(FilteredNavData filteredData,
      Arrayd times, CorrectorSet<adouble>::Ptr correctorSet,
      LevmarSettings settings) : _filteredRawData(filteredData) {
  ENTERSCOPE("CalibratedNavData");
  _correctorSet = (bool(correctorSet)? correctorSet : CorrectorSet<adouble>::Ptr(new DefaultCorrectorSet<adouble>()));
  if (times.empty()) {
    times = filteredData.makeCenteredX();
  }
  Objf objf(filteredData, _correctorSet, times);
  SCOPEDMESSAGE(INFO, stringFormat("Objf dimensions: %d", objf.outDims()));
  Arrayad initParams(_correctorSet->paramCount());
  _correctorSet->initialize(initParams.ptr());
  LevmarState state(initParams.map<double>([&](adouble x) {return x.getValue();}));
  state.minimize(settings, objf);
  _optimalCalibrationParameters = state.getXArray(true);
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



}
