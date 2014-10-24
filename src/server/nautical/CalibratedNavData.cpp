/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/math/ADFunction.h>
#include <server/nautical/CalibratedNavData.h>
#include <server/math/nonlinear/Levmar.h>
#include <server/math/armaadolc.h>


namespace sail {

namespace {
  class Objf : public AutoDiffFunction {
   public:
    static constexpr int eqsPerComparison = 4;

    Objf(FilteredNavData data, CorrectorSet<adouble>::Ptr corr, Arrayd times);

    int inDims() {
      return _corr->paramCount();
    }

    int outDims() {
      return eqsPerComparison*_times.size();
    }

    void evalAD(adouble *Xin, adouble *Fout);
   private:
    FilteredNavData _data;
    CorrectorSet<adouble>::Ptr _corr;
    Arrayd _times, _weights;
    LineKM _sampling;

    CalibratedValues<adouble> compute(int time, adouble *parameters);
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
                  Velocity<adouble>::knots(_data.watSpeed().get(index).knots()),
                  Angle<adouble>::degrees(_data.awa().get(index).degrees()),
                  Velocity<adouble>::knots(_data.aws().get(index).knots()));
  }

  Objf::Objf(FilteredNavData data, CorrectorSet<adouble>::Ptr corr,
    Arrayd times) :
      _data(data), _corr(corr),
      _times(times),
      _weights(data.magHdg().interpolateLinearDerivative(times).map<double>([&](Angle<double> x) {return x.degrees();})),
      _sampling(data.sampling()) {}

  void Objf::evalAD(adouble *Xin, adouble *Fout) {
    assert(_times.size() == _weights.size());
    for (int i = 0; i < _times.size(); i++) {
      int index = int(floor(_sampling.inv(_times[i])));
      CalibratedValues<adouble> from = compute(index, Xin);
      CalibratedValues<adouble> to = compute(index + 1, Xin);
      evalDif(_weights[i], from, to, Fout + i*eqsPerComparison);
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
      CorrectorSet<adouble>::Ptr correctorSet, Arrayd times,
      LevmarSettings settings) : _filteredRawData(filteredData) {
  _correctorSet = (bool(correctorSet)? correctorSet : CorrectorSet<adouble>::Ptr(new DefaultCorrectorSet<adouble>()));
  if (times.empty()) {
    times = filteredData.makeCenteredX();
  }
  Objf objf(filteredData, _correctorSet, times);
  Arrayad initParams(_correctorSet->paramCount());
  _correctorSet->initialize(initParams.ptr());
  LevmarState state(initParams.map<double>([&](adouble x) {return x.getValue();}));
  state.minimize(settings, objf);
  _optimalCalibrationParameters = state.getXArray(true);
}



}
