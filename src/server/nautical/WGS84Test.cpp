/*
 *  Created on: 11 févr. 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "Ecef.h"
#include "WGS84.h"
#include <cmath>
#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
#include <gtest/gtest.h>
#include <server/common/Function.h>
#include <server/common/LineKM.h>
#include <server/common/logging.h>
#include <server/common/math.h>
#include <server/common/string.h>

using namespace sail;

// Check that the output XYZ position of this function is reasonable
TEST(wgs84Test, CheckItIsReasonableTest) {
  Angle<double> lon = Angle<double>::radians(0.3*M_PI);
  Angle<double> lat = Angle<double>::radians(0.4*M_PI);
  Length<double> altitude = Length<double>::meters(0.0);

  double circumferenceEarthMetres = 40*1.0e6; // 40 000 km
  double radius = circumferenceEarthMetres/(2.0*M_PI);

  Length<double> xyzl[3];
  WGS84<double>::toXYZ(GeographicPosition<double>(lon, lat, altitude), xyzl);
  double xyz[3];
  for (int i = 0; i < 3; i++) {
    xyz[i] = xyzl[i].meters();
  }
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
        WGS84<double>::toXYZLocal(lon, lat,
            altitudeMetres, xyz, nullptr, nullptr);

        double ex, ey, ez;
        lla2ecefJustForReference(lon, lat, altitudeMetres, ex, ey, ez);
        EXPECT_NEAR(ex, xyz[0], 1.0e-5);
        EXPECT_NEAR(ey, xyz[1], 1.0e-5);
        EXPECT_NEAR(ez, xyz[2], 1.0e-5);

        Length<double> xyz2[3];
        WGS84<double>::toXYZ(GeographicPosition<double>(Angle<double>::radians(lon),
                             Angle<double>::radians(lat),
                             Length<double>::meters(altitudeMetres)), xyz2);
        EXPECT_NEAR(ex, xyz2[0].meters(), 1.0e-5);
        EXPECT_NEAR(ey, xyz2[1].meters(), 1.0e-5);
        EXPECT_NEAR(ez, xyz2[2].meters(), 1.0e-5);
      }
    }
  }
}

class WGS84TestFun : public Function {
 public:
  int inDims() {return 3;}
  int outDims() {return 3;}
  void eval(double *Xin, double *Fout, double *Jout) {
    typedef WGS84<double> Mapper;
    Mapper::toXYZWithJ(Xin[0], Xin[1], Xin[2], Fout, Jout);
  }
};


// Validate the Jacobian, both for radians and degrees
TEST(wgs84Test, JacobianRadDegTest) {
  double lonsAndLats[3] = {-0.2, 0.2, 0.5};
  double altitudes[3] = {0, 12, 144};

  WGS84TestFun fun;
  for (int i = 0; i < 3; i++) {
    double lon = lonsAndLats[i]*M_PI;
    for (int j = 0; j < 3; j++) {
      double lat = lonsAndLats[j]*M_PI;
      for (int k = 0; k < 3; k++) {
        double altitudeMetres = altitudes[k];
        double X[3] = {lon, lat, altitudeMetres};
        double J[9], JNum[9], F[3];

        fun.eval(X, F, J);
        fun.evalNumericJacobian(X, JNum);
        for (int i = 0; i < 9; i++) {
          const bool verbose = false;
          if (verbose) {
            LOG(INFO) << "\n\n" << EXPR_AND_VAL_AS_STRING(i) << '\n';
            LOG(INFO) << EXPR_AND_VAL_AS_STRING(JNum[i])
                << "\n" << EXPR_AND_VAL_AS_STRING(J[i]) << '\n';
          }
          double reldif =
              (J[i] - JNum[i])/(std::abs(0.5*(JNum[i] + J[i])) + 1.0e-6);

          // Be quite tolerant here because numeric differentiation is dirty:
          EXPECT_NEAR(reldif, 0, 2.0e-3);
        }
      }
    }
  }
}


// Check that the direction is correct at the equator
TEST(wgs84Test, DirTest001) {
  double lat = 0.0*M_PI;
  const int sign[4] = {1, -1, -1, 1};
  const int courseCount = 7;
  LineKM courses(0, courseCount-1, -323, 1044);

  for (int K = 0; K < 4; K++) {
    bool even = (K % 2 == 0);
    double lon = 0.5*M_PI*K;
    double altitudeMetres = 0.0;


    for (int i = 0; i < courseCount; i++) {
      double course = Angle<double>::degrees(courses(i)).radians();

      Length<double> xyz[3];
      double dirXyz[3];
      WGS84<double>::posAndDirToXYZ(GeographicPosition<double>(Angle<double>::radians(lon),
          Angle<double>::radians(lat),
          Length<double>::meters(altitudeMetres)), Angle<double>::radians(course), xyz, dirXyz);

      double sinc = sign[K]*sin(course);

      EXPECT_NEAR(dirXyz[0], (even? 0.0 : sinc), 1.0e-5);
      EXPECT_NEAR(dirXyz[1], (even? sinc : 0.0), 1.0e-5);
      EXPECT_NEAR(dirXyz[2], cos(course), 1.0e-5);
    }
  }
}

// Check that the direction is correct close to the poles.
// Here the direction in Z should be close to 0.
TEST(wgs84Test, DirTest002) {
  // Exactly at the pole, the dir vector may be undefined, resulting in
  // division by 0 => nan. Therefore, choose a point close to the pole.
  const double d = 1.0e-6;
  const double a = 0.5*M_PI - d;

  LineKM lat(0.0, 1.0, -a, a);
  const int courseCount = 7;
  LineKM courses(0, courseCount-1, -323, 1044);

  const int lonCount = 7;
  LineKM lons(0.0, lonCount-1, -3.4, 4.9);

  for (int j = 0; j < lonCount; j++) {
    double lon = lons(j);
    for (int i = 0; i < courseCount; i++) {
      double course = Angle<double>::degrees(courses(i)).radians();
      for (int i = 0; i < 2; i++) {
        double dirXyz[3];
        Length<double> xyz[3];
        WGS84<double>::posAndDirToXYZ(GeographicPosition<double>(Angle<double>::radians(lon),
            Angle<double>::radians(lat(i)),
            Length<double>::meters(0.0)), Angle<double>::radians(course), xyz, dirXyz);
        EXPECT_NEAR(dirXyz[2], 0.0, 1.0e-5);
      }
    }
  }
}

// Illustrate the meaning of dlon and dlat of toXYZLocal
// Shows how it is connected to toXYZWithJ
TEST(wgs84Test, MeaningOfDlonDlatTest) {
  const int counts = 4;
  LineKM lons(0, counts-1, -3.3, 4);
  LineKM lats(0, counts-1, -2, 4.4);
  LineKM alts(0, counts-1, 0.0, 3000);
  for (int i = 0; i < counts; i++) {
    double lon = lons(i);
    for (int j = 0; j < counts; j++) {
      double lat = lats(j);
      for (int k = 0; k < counts; k++) {
        double alt = alts(k);

        double xyz[3], J[9];
        WGS84<double>::toXYZWithJ(lon, lat, alt, xyz, J);

        double xyz2[3], dlon, dlat;
        WGS84<double>::toXYZLocal(lon, lat, alt,
            xyz2, &dlon, &dlat);

        // Relative errors
        EXPECT_NEAR(1.0, norm<double>(3, J + 0)/dlon, 1.0e-4);
        EXPECT_NEAR(1.0, norm<double>(3, J + 3)/dlat, 1.0e-4);

        // ..and an extra test here just to check that xyz
        // and xyz2 are the same (we tested this before too)
        EXPECT_NEAR((normdif<double, 3>(xyz, xyz2)), 0.0, 1.0e-4);


        double xyz3[3], dlon3, dlat3;
                WGS84<double>::toXYZLocal(lon, lat, NAN,
                    xyz3, &dlon3, &dlat3);
        EXPECT_TRUE(std::isfinite(dlon3));
        EXPECT_TRUE(std::isfinite(dlat3));
      }
    }
  }
}

