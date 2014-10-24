/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/CalibratedNavData.h>

namespace sail {

namespace {
  class Objf : public AutoDiffFunction {
   public:
    static constexpr int eqsPerComparison = 4;

    Objf(FilteredNavData data, CorrectorSet<adouble>::Ptr corr, Arrayd times = Arrayd());

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
        HorizontalMotion<adouble>::polar(_data.gpsSpeed().get(index),
            _data.gpsBearing().get(index));

    return CalibratedValues<adouble>(_corr,
                  parameters,
                  gpsMotion,
                  _data.magHdg().get(index),
                  _data.watSpeed().get(index),
                  _data.awa().get(index),
                  _data.aws().get(index));
  }

  Objf::Objf(FilteredNavData data, CorrectorSet<adouble>::Ptr corr,
    Arrayd times) :
      _data(data), _corr(corr),
      _times(times),
      _weights(data.magHdg().interpolateLinearDerivative(times)),
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
    dst[0] = w*(a.trueWind[0] - b.trueWind[0]);
    dst[1] = w*(a.trueWind[1] - b.trueWind[1]);
    dst[2] = w*(a.trueCurrent[0] - b.trueCurrent[0]);
    dst[3] = w*(a.trueCurrent[1] - b.trueCurrent[1]);
  }


}

CalibratedNavData::CalibratedNavData(FilteredNavData filteredData,
      CorrectorSet<adouble>::Ptr correctorSet) {

}



}
