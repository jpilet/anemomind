/*
 * Curve2dFilterTest.cpp
 *
 *  Created on: 30 Mar 2017
 *      Author: jonas
 */

#include <gtest/gtest.h>
#include <server/math/Curve2dFilter.h>
#include <server/common/TimedValue.h>
#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
#include <server/common/ArrayIO.h>
#include <server/common/ArrayBuilder.h>

using namespace sail;
using namespace sail::Curve2dFilter;

Settings makeTestSettings() {
  auto s = Settings();
  s.minimumInlierCount = 1;
  return s;
}

auto offset = TimeStamp::UTC(2017, 3, 30, 17, 37, 0);

template <typename T>
TimedValue<T> tv(TimeStamp t, T x) {
  return TimedValue<T>(t, x);
}

template <typename T>
TimedValue<Vec2<T>> ltv(
    Duration<double> t, T x, T y) {
  return tv(offset + t, Vec2<T>{x, y});
}

void checkCurve(const Results& results) {
  EXPECT_TRUE(results.OK());

  EXPECT_NEAR(results.curve().evaluate(0, offset + 0.0_s).meters(), 0.0, 1.0e-6);
  EXPECT_NEAR(results.curve().evaluate(0, offset + 2.0_s).meters(), 3.0, 1.0e-6);
  EXPECT_NEAR(results.curve().evaluate(0, offset + 4.0_s).meters(), 6.0, 1.0e-6);

  EXPECT_NEAR(results.curve().evaluate(1, offset + 0.0_s).meters(), 2.0, 1.0e-6);
  EXPECT_NEAR(results.curve().evaluate(1, offset + 2.0_s).meters(), 1.5, 1.0e-6);
  EXPECT_NEAR(results.curve().evaluate(1, offset + 4.0_s).meters(), 1.0, 1.0e-6);

  auto motionCurve = results.curve().derivative();

  for (int i = 0; i < 3; i++) {
    auto t = offset + double(i)*2.0_s;
    EXPECT_NEAR(motionCurve.evaluate(0, t).metersPerSecond(),
          1.5, 1.0e-6);
    EXPECT_NEAR(motionCurve.evaluate(1, t).metersPerSecond(),
          -0.25, 1.0e-6);
  }
}

TEST(Curve2dFilterTest, TestWithPositions) {
  for (int k: {1, 2}) {
    Array<TimedValue<Vec2<Length<double>>>> positions{
      ltv(0.0_s, 0.0_m, 2.0_m),
      ltv(4.0_s, 6.0_m, 1.0_m)
    };

    Array<TimedValue<Vec2<Velocity<double>>>> velocities;

    TimeMapper mapper(offset, double(k)*1.0_s, 8);
    Settings settings = makeTestSettings();
    auto results = optimize(mapper, positions, velocities, settings);

    EXPECT_TRUE(results.OK());

    checkCurve(results);
  }
}

TEST(Curve2dFilterTest, TestWithMotions) {
  for (int k: {1, 2}) {
    Array<TimedValue<Vec2<Length<double>>>> positions{
      ltv(0.0_s, 0.0_m, 2.0_m),
    };

    Array<TimedValue<Vec2<Velocity<double>>>> velocities{
      ltv(0.0_s, 1.5_mps, -0.25_mps)
    };

    TimeMapper mapper(offset, double(k)*1.0_s, 8);
    auto results = optimize(mapper, positions, velocities, makeTestSettings());
    checkCurve(results);
  }
}

TEST(Curve2dFilterTest, TestWithOnePositionOutlier) {
  TimeMapper mapper(offset, 2.0_s, 8);
  ArrayBuilder<TimedValue<Vec2<Length<double>>>> positions;
  for (int i = 0; i < 8; i++) {
    auto t = double(i)*2.0_s;
    if (i == 1) {
      positions.add(ltv(t, 100.0_m, -200.0_m));
    } else {
      positions.add(ltv<Length<double>>(
          t, t*1.5_mps, 2.0_m + t*-0.25_mps));
    }
  }
  auto results = optimize(mapper, positions.get(),
      Array<TimedValue<Vec2<Velocity<double>>>>(), makeTestSettings());
  checkCurve(results);
}

