/*
 * GpsUtilsTest.cpp
 *
 *  Created on: May 12, 2016
 *      Author: jonas
 */

#include <gtest/gtest.h>
#include <server/nautical/filters/GpsUtils.h>
#include <server/nautical/WGS84.h>

using namespace sail;

namespace {
  std::string src = "Test source";
  auto offset = TimeStamp::UTC(2015, 5, 12, 13, 45, 0);
  auto degrees = Angle<double>::degrees(1.0);
  auto meters = Length<double>::meters(1.0);
  auto seconds = Duration<double>::seconds(1.0);
  auto knots = Velocity<double>::knots(1.0);

  typedef GeographicPosition<double> GeoPos;
  typedef TimedValue<GeoPos> TimedPosition;
  typedef TimedValue<Angle<double> > TimedAngle;
  typedef TimedValue<Velocity<double> > TimedVelocity;

  auto thePos = GeoPos(34.0*degrees, 23.0*degrees, 0.4*meters);

  std::shared_ptr<Dispatcher> makeDispatcher() {
    auto d = std::make_shared<Dispatcher>();
    d->insertValues<GeoPos>(GPS_POS, src,
        TimedSampleCollection<GeoPos>::TimedVector{
      TimedPosition{offset + 1.0*seconds, thePos}
    });

    d->insertValues<Angle<double> >(GPS_BEARING, src,
        TimedSampleCollection<Angle<double> >::TimedVector{
      TimedAngle{offset + 34.0*seconds, 94.0*degrees}
    });

    d->insertValues<Velocity<double> >(GPS_SPEED, src,
        TimedSampleCollection<Velocity<double> >::TimedVector{
      TimedVelocity{offset + 35.0*seconds, 4.6*knots}
    });
    return d;
  }
}

TEST(GpsUtilsTest, TestSomeFunctions) {
  NavDataset ds(makeDispatcher());

  auto motions = GpsUtils::getGpsMotions(ds);
  EXPECT_EQ(motions.size(), 1);
  auto m = motions[0];
  EXPECT_NEAR((m.time - offset).seconds(), 34.5, 1.0e-6);

  auto em = HorizontalMotion<double>::polar(4.6*knots, 94.0*degrees);

  EXPECT_NEAR(m.value[0].knots(), em[0].knots(), 1.0e-6);
  EXPECT_NEAR(m.value[1].knots(), em[1].knots(), 1.0e-6);

  auto positions = ds.samples<GPS_POS>();
  EXPECT_EQ(positions.size(), 1);

  auto refPos = GpsUtils::getReferencePosition(positions);
  {
    Length<double> xyz[3], xyz2[3];
    WGS84<double>::toXYZ(thePos, xyz);
    WGS84<double>::toXYZ(refPos, xyz2);
    for (int i = 0; i < 3; i++) {
      EXPECT_NEAR(xyz[i].meters(), xyz2[i].meters(), 1.0e-6);
    }
  }

  auto refTime = GpsUtils::getReferenceTime(positions);
  EXPECT_NEAR((refTime - offset).seconds(), 1.0, 1.0e-6);
}
