/*
 * SplineGpsFilterTest.cpp
 *
 *  Created on: 31 Oct 2016
 *      Author: jonas
 */


#include <server/nautical/filters/SplineGpsFilter.h>
#include <gtest/gtest.h>
#include <server/common/ArrayBuilder.h>
#include <server/nautical/WGS84.h>


using namespace sail;

auto offset = TimeStamp::UTC(2016, 11, 1, 8, 17, 0.0);



namespace {
  GeographicPosition<double> makePos(double x, double y) {
    auto step = (1.0/(60*1852))*1.0_deg;
    return GeographicPosition<double>(45.0_deg + x*step, y*step, 0.0_m);
  }

  TimedValue<GeographicPosition<double>> makeTPos(
    double t, double x, double y) {
    return TimedValue<GeographicPosition<double>>(
        offset + t*1.0_s,
        makePos(x, y));
  }

  void testGpsPos(
      const SplineGpsFilter::EcefCurve &curve,
      const TimedValue<GeographicPosition<double>> &pos,
      double tolMeters) {

    auto p = curve.evaluateGeographicPosition(pos.time);
    auto d = distance(p, pos.value);
    std::cout << "The distance is " << d.meters() << std::endl;
    EXPECT_NEAR(d.meters(), 0.0, tolMeters);
/*

    double tol = tolMeters/(60.0*1852);
    EXPECT_NEAR(p.lat().degrees(), pos.value.lat().degrees(), tol);
    EXPECT_NEAR(p.lon().degrees(), pos.value.lon().degrees(), tol);
    EXPECT_NEAR(p.alt().meters(), pos.value.alt().meters(), tolMeters);*/
  }

  TimedValue<HorizontalMotion<double>> makeTMot(
      double t, double x, double y) {
    return TimedValue<HorizontalMotion<double>>(
        offset + t*1.0_s,
        HorizontalMotion<double>(x*1.0_mps, y*1.0_mps));
  }
}

TEST(SplineGpsFilter, TestIt) {
  Array<TimedValue<GeographicPosition<double>>> pos{
    makeTPos(0.0, 0.0, 0.0),
    makeTPos(1.0, 1.0, 0.0),
    makeTPos(2.0, 2.0, 0.0)
  };

  SplineGpsFilter::Settings settings;
  settings.samplingPeriod = 1.0_s;
  auto curves = segmentAndFilter(pos, {}, settings);
  EXPECT_EQ(1, curves.size());
  testGpsPos(curves[0], makeTPos(0.0, 0.0, 0.0), 0.001);
  testGpsPos(curves[0], makeTPos(1.0, 1.0, 0.0), 0.001);
  testGpsPos(curves[0], makeTPos(2.0, 2.0, 0.0), 0.001);
}

TEST(SplineGpsFilter, TestIt2) {
  Array<TimedValue<GeographicPosition<double>>> pos{
    makeTPos(0.0, 0.0, 0.0),
    makeTPos(1.0, 1.0, 0.0),
    makeTPos(2.0, 302344.0, 0.0),
    makeTPos(3.0, 3.0, 0.0),
    makeTPos(4.0, 4.0, 0.0),
  };

  SplineGpsFilter::Settings settings;
  auto curves = segmentAndFilter(pos, {}, settings);
  EXPECT_EQ(1, curves.size());
  testGpsPos(curves[0], makeTPos(2.0, 2.0, 0.0), 0.001);
}

TEST(SplineGpsFilter, TestIt3) {
  ArrayBuilder<TimedValue<GeographicPosition<double>>> dst0;
  for (int i = 0; i < 31; i++) {
    dst0.add(makeTPos(i, i == 15? 1234234.034 : i, 0));
  }
  auto pos = dst0.get();

  SplineGpsFilter::Settings settings;
  auto curves = segmentAndFilter(pos, {}, settings);
  EXPECT_EQ(1, curves.size());
  testGpsPos(curves[0], makeTPos(15.0, 15.0, 0.0), 0.01);
}

TEST(SplineGpsFilter, TestIt4) {
  ArrayBuilder<TimedValue<GeographicPosition<double>>> pos0;
  for (int i = 0; i < 30; i++) {
    pos0.add(makeTPos(i, 0.0, 9.0 + 2.0*i));
  }

  auto pos = pos0.get();

  //auto mot = mot0.get();
  SplineGpsFilter::Settings settings;
  auto curves = segmentAndFilter(pos, {}, settings);
  EXPECT_EQ(1, curves.size());
  testGpsPos(curves[0], makeTPos(15.0, 0.0, 39.0), 0.001);

  auto motion = curves[0].evaluateHorizontalMotion(offset + 15.0_s);
  EXPECT_NEAR(motion[0].metersPerSecond(), 0.0, 0.001);
  EXPECT_NEAR(motion[1].metersPerSecond(), 2.0, 0.001);
}

TEST(SplineGpsFilter, TestIt5) {
  Array<TimedValue<GeographicPosition<double>>> pos{
    makeTPos(0.0, 1.0, 0.0),
    makeTPos(1.0, 3.0, 0.0),
    makeTPos(2.0, 5.0, 0.0)
  };

  SplineGpsFilter::Settings settings;
  settings.wellPosednessReg = 1.0e-5;
  settings.regWeight = 0.0;
  settings.samplingPeriod = 0.5_s;
  auto curves = segmentAndFilter(pos, {}, settings);
  EXPECT_EQ(curves.size(), 1);
  auto curve = curves[0];
  auto k = curves[0].evaluateHorizontalMotion(offset + 0.5_s);
  testGpsPos(curve, makeTPos(0.0, 1.0, 0.0), 0.001);
  testGpsPos(curve, makeTPos(1.0, 3.0, 0.0), 0.001);
}

TEST(SplineGpsFilter, TestIt6) {
  ArrayBuilder<TimedValue<GeographicPosition<double>>> pos0;
  int n = 30;
  double offset = 9.0;
  for (int i = 0; i < n; i++) {
    pos0.add(makeTPos(i, 0.0, offset + 2*i));
  }

  auto pos = pos0.get();

  //auto mot = mot0.get();
  SplineGpsFilter::Settings settings;
  settings.regWeight = 1.0;
  settings.wellPosednessReg = 1.0e-6;
  auto curves = segmentAndFilter(pos, {}, settings);
  EXPECT_EQ(1, curves.size());

  int middle = double(n)/2.0;
  testGpsPos(curves[0], makeTPos(middle,
      0.0, offset + 2.0*middle), 0.01);
}

TEST(SplineGpsFilter, TestIt7) {
  Array<TimedValue<GeographicPosition<double>>> pos{
    makeTPos(0.0, 3.0, 4.0)
  };
  ArrayBuilder<TimedValue<HorizontalMotion<double>>> mot;
  for (int i = 0; i < 9; i++) {
    mot.add(makeTMot(i, 0.5, 1.0));
  }

  SplineGpsFilter::Settings settings;
  auto curves = segmentAndFilter(pos, mot.get(), settings);
  auto curve = curves[0];
  EXPECT_EQ(1, curves.size());

  auto t = 4.0;
  auto X = 3.0 + 0.5*t;
  auto Y = 4.0 + 1.0*4;
  testGpsPos(curve, makeTPos(t, X, Y), 0.001);
}

