/*
 * CurrentCalibTest.cpp
 *
 *  Created on: Apr 15, 2016
 *      Author: jonas
 */

#include <gtest/gtest.h>
#include <server/nautical/calib/CornerCalibTestData.h>
#include <server/nautical/calib/CurrentCalib.h>

namespace {
  using namespace sail;
  using namespace sail::CornerCalibTestData;

  auto knots = Velocity<double>::knots(1.0);

  HorizontalMotion<double> vecToMotion(const Eigen::Vector2d &x) {
    return HorizontalMotion<double>(x[0]*knots, x[1]*knots);
  }

  NavDataset makeDatasetFromTestSamples(const Array<TestSample> &samples) {
    //auto d = new Dispatcher();

    TimedSampleCollection<Angle<double> >::TimedVector magHeading, gpsBearing;
    TimedSampleCollection<Velocity<double> >::TimedVector watSpeed, gpsSpeed;

    auto seconds = Duration<double>::seconds(1.0);
    auto offset = TimeStamp::UTC(2016, 4, 21, 15, 21, 0);


    for (int i = 0; i < samples.size(); i++) {
      TestSample x = samples[i];
      auto t = offset + double(i)*seconds;

      auto watMotion = x.corruptedMotionOverWaterVec();
      auto gpsMotion = x.boatMotionVec;

      magHeading.push_back(TimedValue<Angle<double> >(t, watMotion.angle()));
      watSpeed.push_back(TimedValue<Velocity<double> >(t, watMotion.norm()));
      gpsBearing.push_back(TimedValue<Angle<double> >(t, gpsMotion.angle()));
      gpsSpeed.push_back(TimedValue<Velocity<double> >(t, gpsMotion.norm()));
    }

    auto src = "testsample";

    auto d = std::make_shared<Dispatcher>();
    d->insertValues<Angle<double> >(MAG_HEADING, src, magHeading);
    d->insertValues<Velocity<double> >(WAT_SPEED, src, watSpeed);
    d->insertValues<Angle<double> >(GPS_BEARING, src, gpsBearing);
    d->insertValues<Velocity<double> >(GPS_SPEED, src, gpsSpeed);
    return NavDataset(d).fitBounds();
  }
}

TEST(CurrentCalib, BasicTest) {
  auto corrupt = sail::CornerCalibTestData::getDefaultCorruptParams();
  auto samples = sail::CornerCalibTestData::makeTestSamples(corrupt);
  auto ds = makeDatasetFromTestSamples(samples);

  CurrentCalib::Settings settings;
  settings.windowSize = Duration<double>::seconds(16);
  settings.samplingPeriod = Duration<double>::seconds(1.0);
  auto current = CurrentCalib::computeCorrectedCurrent(ds, settings);
  auto gt = CornerCalibTestData::getTrueConstantCurrent();
  Velocity<double> sumDifs = Velocity<double>::knots(0.0);
  for (auto c: current) {
    HorizontalMotion<double> dif = gt - c.value;
    sumDifs += dif.norm();
  }
  Velocity<double> meanError = (1.0/current.size())*sumDifs;
  EXPECT_LT(meanError.knots(), 0.1);
}
