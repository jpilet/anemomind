/*
 *  Created on: 11 févr. 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <cmath>
#include "WGS84.h"
#include <server/common/math.h>
#include "Ecef.h"
#include <gtest/gtest.h>
#include <server/common/string.h>
#include <server/common/Function.h>

using namespace sail;

// Check that the output XYZ position of this function is reasonable
TEST(wgs84Test, CheckItIsReasonableTest) {
  double lon = 0.3*M_PI;
  double lat = 0.4*M_PI;
  double altitudeMetres = 0.0;

  double circumferenceEarthMetres = 40*1.0e6; // 40 000 km
  double radius = circumferenceEarthMetres/(2.0*M_PI);

  double xyz[3];
  Wgs84<double, false>::toXYZ(lon, lat, altitudeMetres, xyz);
  double distFromCog = norm(3, xyz);
  double tol = 0.1;
  EXPECT_LE((1.0 - tol)*radius, distFromCog);
  EXPECT_LE(distFromCog, (1.0 + tol)*radius);
}

// Test if this function does the same thing as the ECEF library. It should.
// Also compare it to the implementation of the NmeaParser library.
TEST(wgs84Test, CompareToECEFTest) {
  double lonsAndLats[3] = {-0.2, 0.2, 0.5};
  double altitudes[3] = {0, 12, 144};

  for (int i = 0; i < 3; i++) {
    double lon = lonsAndLats[i]*M_PI;
    for (int j = 0; j < 3; j++) {
      double lat = lonsAndLats[j]*M_PI;
      for (int k = 0; k < 3; k++) {
        double altitudeMetres = altitudes[k];
        double xyz[3];
        Wgs84<double, false>::toXYZCopiedFromNmeaParserLib(lon, lat, altitudeMetres, xyz, nullptr, nullptr);

        double ex, ey, ez;
        lla2ecef(lon, lat, altitudeMetres, ex, ey, ez);
        EXPECT_NEAR(ex, xyz[0], 1.0e-5);
        EXPECT_NEAR(ey, xyz[1], 1.0e-5);
        EXPECT_NEAR(ez, xyz[2], 1.0e-5);

        double xyz2[3];
        Wgs84<double, false>::toXYZ(lon, lat, altitudeMetres, xyz2);
        EXPECT_NEAR(ex, xyz2[0], 1.0e-5);
        EXPECT_NEAR(ey, xyz2[1], 1.0e-5);
        EXPECT_NEAR(ez, xyz2[2], 1.0e-5);      }
    }
  }
}

class Wgs84TestFun : public Function {
 public:
  int inDims() {return 3;}
  int outDims() {return 3;}
  void eval(double *Xin, double *Fout, double *Jout);
 private:
};

void Wgs84TestFun::eval(double *Xin, double *Fout, double *Jout) {
  typedef Wgs84<double, false> Mapper;
  Mapper::toXYZWithJ(Xin[0], Xin[1], Xin[2], Fout, Jout);
}

TEST(wgs84Test, JacobianTest) {
  double lonsAndLats[3] = {-0.2, 0.2, 0.5};
  double altitudes[3] = {0, 12, 144};

  for (int i = 0; i < 3; i++) {
    double lon = lonsAndLats[i]*M_PI;
    for (int j = 0; j < 3; j++) {
      double lat = lonsAndLats[j]*M_PI;
      for (int k = 0; k < 3; k++) {
        double altitudeMetres = altitudes[k];
        double X[3] = {lon, lat, altitudeMetres};
        double J[9], JNum[9], F[3];
        Wgs84TestFun fun;
        fun.eval(X, F, J);
        fun.evalNumericJacobian(X, JNum);
        for (int i = 0; i < 9; i++) {
          double reldif = (J[i] - JNum[i])/(JNum[i] + 1.0e-5);
          EXPECT_NEAR(reldif, 0, 1.0e-3);
        }
      }
    }
  }
}


// Check that the direction is correct
TEST(wgs84Test, DirTest) {
  double lon = 0.3*M_PI;
  double lat = 0.4*M_PI;
  double altitudeMetres = 30.0;

  // If we move in direction of 45 degrees, our course is north-east
  double dir = deg2rad(45);

  double xyz[3], dirXyz[3];
  Wgs84<double, false>::posAndDirToXYZ(lon, lat, altitudeMetres, dir, xyz, dirXyz);

  double xyzh[3];
  double h = 1.0e-9;
  lon += h;
  lat += h;
  Wgs84<double, false>::toXYZ(lon, lat, altitudeMetres, xyzh);
  double displacement[3];
  sub<double, 3>(xyzh, xyz, displacement);
  normalizeInPlace<double>(3, displacement);
  for (int i = 0; i < 3; i++) {
    double dif = std::abs(displacement[i] - dirXyz[i]);
    EXPECT_NEAR(displacement[i], dirXyz[i], 1.0e-5);
  }
}

