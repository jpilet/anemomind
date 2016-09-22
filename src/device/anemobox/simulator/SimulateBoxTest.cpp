#include <gtest/gtest.h>
#include <device/anemobox/simulator/SimulateBox.h>
#include <server/nautical/calib/Calibrator.h>

using namespace sail;

namespace {

  template <typename T>
  void pt(std::shared_ptr<ReplayDispatcher> dst, DataCode c, Duration<double> localTime, T x) {
    auto start = TimeStamp::UTC(2016, 3, 24, 18, 10, 0);
    const char *name = "test";
    TimeStamp t = start + localTime;

    if (dst->currentTime().defined()) {
      EXPECT_LE(dst->currentTime(), t);
    }

    dst->publishTimedValue<T>(c, name, TimedValue<T>(t, x));
  }
}

// This test checks that the true wind estimator will get called when it should,
// when we are doing a replay.
TEST(SimulateBox, Replay) {

  
  std::shared_ptr<ReplayDispatcher> rd = std::make_shared<ReplayDispatcher>();
  NavDataset original(std::static_pointer_cast<Dispatcher>(rd));

  /*    AWA, AWS, GPS_SPEED, GPS_BEARING,
      WAT_SPEED, MAG_HEADING*/
  auto knots = Velocity<double>::knots(1.0);
  auto degrees = Angle<double>::degrees(1.0);
  auto ms = Duration<double>::milliseconds(1.0);

  typedef Angle<double> A;
  typedef Velocity<double> V;

  // First a dense batch of values.
  pt<A>(rd, AWA, 0.0*ms, 3.0*degrees); // Compute first triggered here to occur at 20 ms from now.
  pt<V>(rd, AWS, 1.0*ms, 12.0*knots);
  pt<V>(rd, GPS_SPEED, 2.0*ms, 4.0*knots);
  pt<A>(rd, GPS_BEARING, 3.0*ms, 34.0*degrees);
  pt<V>(rd, WAT_SPEED, 4.0*ms, 4.0*knots);
  pt<A>(rd, MAG_HEADING, 4.0*ms, 35.0*degrees);
  pt<A>(rd, AWA, 4.0*ms, 3.0*degrees);
  pt<V>(rd, AWS, 5.0*ms, 12.0*knots);
  pt<V>(rd, GPS_SPEED, 5.0*ms, 4.0*knots);
  pt<A>(rd, GPS_BEARING, 5.0*ms, 34.0*degrees);
  pt<V>(rd, WAT_SPEED, 6.0*ms, 4.0*knots);
  pt<A>(rd, MAG_HEADING, 7.0*ms, 35.0*degrees);

  // Compute will be called here, at 20 ms

  // Gap of 3000 - 7 ms = 2993 ms

  pt<A>(rd, AWA, 3000.0*ms, 3.0*degrees); // <-- 13th value. Schedule compute at 3020 ms

  // This is a stack of the number of values that the destination dispatcher
  // will contain when it is called. So for the first call, it will contain 12
  // values, and for the second call it will contain 13.
  std::vector<int> expectedCounts{13, 12};

  // Let's create a default calibration to allow the true wind estimator
  // to do its job.
  std::stringstream calibFile;
  Calibrator calibrator;
  calibrator.saveCalibration(&calibFile);
  calibFile.seekg(0, std::ios::beg);

  NavDataset simulated = SimulateBox(calibFile, original);

  EXPECT_EQ(0, original.samples<TWDIR>().size());
  EXPECT_EQ(1, simulated.samples<TWDIR>().size());
}
