/*
 *  Created on: 2014-07-03
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "WaterCalib.h"
#include <server/math/armaadolc.h>
#include <server/math/ADFunction.h>

namespace sail {

namespace {
  Array<Nav> getDownwindNavs(Array<Nav> navs) {
    return navs.slice([&](const Nav &n) {
      return cos(n.externalTwa()) < 0;
    });
  }
}

WaterCalib::WaterCalib(const HorizontalMotionParam &param) : _param(param) {}

void WaterCalib::initialize(double *outParams) const {
  wcK(outParams) = SpeedCalib<double>::initK();
  wcM(outParams) = 0;
  wcC(outParams) = 0;
  wcAlpha(outParams) = 0;
  magOffset(outParams) = 0;
  _param.initialize(outParams + 5);
}

namespace {
  class WaterCalibObjf : public AutoDiffFunction {
   public:
    WaterCalibObjf(const WaterCalib &calib, Array<Nav> navs) : _calib(calib), _navs(navs) {}
    int inDims() {
      return _calib.paramCount();
    }

    int outDims() {
      return 2*_navs.size();
    }

    void evalAD(adouble *Xin, adouble *Fout);
   private:
    Array<Nav> _navs;
    const WaterCalib &_calib;
  };

  void WaterCalibObjf::evalAD(adouble *Xin, adouble *Fout) {
    //Arrayad X(inDims(), Xin);
    SpeedCalib<adouble> sc = _calib.makeSpeedCalib<adouble>(Xin);
  }

}

Arrayd WaterCalib::optimize(Array<Nav> allnavs) const {
  Arrayd X(paramCount());
  Array<Nav> navs = getDownwindNavs(allnavs);
  WaterCalibObjf objf(*this, navs);
}


} /* namespace mmm */
