/*
 *  Created on: 2014-03-24
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "GeoCalc.h"
#include <server/common/math.h>
#include <server/math/nonlinear/Levmar.h>
#include <server/math/nonlinear/LevmarSettings.h>
#include <server/math/ADFunction.h>
#include <server/nautical/WGS84.h>
#include <server/common/string.h>
#include <server/common/ArrayIO.h>


namespace sail {

namespace {
  class GeoPosObjf : public AutoDiffFunction {
   public:
    GeoPosObjf(double *XYZ) : _XYZ(XYZ) {}
    int inDims() {return 3;}
    int outDims() {return 3;}
    void evalAD(adouble *Xin, adouble *Fout);
   private:
    double *_XYZ;
  };

  void GeoPosObjf::evalAD(adouble *Xin, adouble *Fout) {
    GeographicPosition<adouble> pos(Angle<adouble>::radians(Xin[0]),
        Angle<adouble>::radians(Xin[1]),
        Length<adouble>::meters(Xin[2]));
    Length<adouble> XYZ[3];
    WGS84<adouble>::toXYZ(pos, XYZ);
    for (int i = 0; i < 3; i++) {
      Fout[i] -= XYZ[i].meters() - _XYZ[i];
    }
  }

}

GeographicPosition<double> toGeographicPosition(Length<double> *XYZ) {
  double X = XYZ[0].meters();
  double Y = XYZ[1].meters();
  double Z = XYZ[2].meters();
  double XYZm[3] = {X, Y, Z};

  double xydist = sqrt(X*X + Y*Y);

  double approxRad = 4.0e7/(2.0*M_PI);
  double approxLat = atan2(Z, xydist);
  double approxLon = atan2(Y, X);
  double approxAlt = norm<double>(3, XYZm) - approxRad;

  double lonLatAlt[3] = {approxLon, approxLat, approxRad};


  Arrayd params(3, lonLatAlt);

  GeoPosObjf objf(XYZm);
  LevmarSettings settings;
  settings.maxiter = 30;
  LevmarState state(params);
  state.minimize(settings, objf);
  Arrayd final = state.getXArray();
  return GeographicPosition<double>(Angle<double>::radians(final[0]),
                                    Angle<double>::radians(final[1]),
                                    Length<double>::meters(final[2]));
}

} /* namespace sail */
