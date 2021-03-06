#include <device/Arduino/libraries/TrueWindEstimator/TrueWindEstimator.h>

#include <device/Arduino/libraries/TrueWindEstimator/InstrumentFilter.h>
#include <gtest/gtest.h>
#include <server/common/Env.h>
#include <server/common/logging.h>
#include <server/nautical/logimport/LogLoader.h>
#include <sstream>
#include <algorithm>
#include <string>
#include <server/transducers/Transducer.h>

using std::string;
using namespace sail;

using namespace NavCompat;

TEST(TrueWindEstimatorTest, SmokeTest) {
  NavDataset navs = LogLoader::loadNavDataset(
      string(Env::SOURCE_DIR) + string("/datasets/tinylog.txt"));

  CHECK_LT(0, getNavSize(navs));

  double parameters[TrueWindEstimator::NUM_PARAMS];
  TrueWindEstimator::initializeParameters(parameters);

  auto trueWind = TrueWindEstimator::computeTrueWind
    <double,ServerFilter>(parameters, makeFilter(navs));

  LOG(INFO) << trueWind[0].knots() << ", " << trueWind[1].knots();
}

TEST(TrueWindEstimatorTest, ManuallyCheckedDataTest) {
  // Sailing upwind. True wind: 222 = 198 + 24, at about 16.2 knots.
  // Existing onboard instruments said:
  // "$IIMWV,024,T,16.2,N,A*16
  const char *nmeaData =
    "$IIRMC,111038,A,4614.021,N,00610.335,E,05.8,196,110708,,,A*49"
    "$IIVHW,,,198,M,05.6,N,,*67"
    "$IIVWR,018,R,21.6,N,,,,*6D"
    "$IIHDG,198,,,,*57"
    "$IIMWV,017,R,21.5,N,A*13"
    "$IIRMC,111039,A,4614.021,N,00610.335,E,05.8,196,110708,,,A*49";
  std::stringstream stream(nmeaData);
  LogLoader loader;
  loader.loadNmea0183(&stream);
  auto navs = loader.makeNavDataset();
  auto navs0 = makeArray(navs);

  EXPECT_TRUE(navs0.hasData());

  // A nav is generated for every GPS position, so we will get two navs.
  EXPECT_EQ(2, getNavSize(navs));

  double parameters[TrueWindEstimator::NUM_PARAMS];
  TrueWindEstimator::initializeParameters(parameters);

  auto trueWind = TrueWindEstimator::computeTrueWind
    <double, ServerFilter>(parameters, makeFilter(navs));

  // Comparing TWDIR
  EXPECT_NEAR(22 + 198, calcTwdir(trueWind).degrees(), 5);

  EXPECT_NEAR(16, trueWind.norm().knots(), 1);
}

namespace {
  HorizontalMotion<double> estimateTrueWindUsingEstimator(const Nav &nav) {
    double parameters[TrueWindEstimator::NUM_PARAMS];
    TrueWindEstimator::initializeParameters(parameters);
    return TrueWindEstimator::computeTrueWind
      <double>(parameters, nav);
  }

  Angle<double> getMedianAbsValue(Array<Angle<double> > difs0) {
    Array<Angle<double> > difs = transduce(difs0, trMap([&](Angle<double> x) {
      return fabs(x);}), IntoArray<Angle<double>>());
    std::sort(difs.begin(), difs.end());
    return difs[difs.size()/2];
  }
}

TEST(TrueWindEstimatorTest, TWACompare) {
  const int dsCount = 2;
  const std::string ds[2] = {string("/datasets/Irene/2008/regate_28_mai_08/IreneLog.txt"),
                             string("/datasets/psaros33_Banque_Sturdza/2014/20140627/NMEA0006.TXT")};

  for (int i = 0; i < dsCount; i++) {
    auto navs = LogLoader::loadNavDataset(string(Env::SOURCE_DIR) + ds[i]);

    navs = sliceTo(navs, 3000);
    int count = getNavSize(navs);

    EXPECT_LE(1000, count);
    Angle<double> tol = Angle<double>::degrees(10.0);
    int counter = 0;

    Array<Angle<double> > difs(count);
    for (int i = 0; i < count; i++) {
      Nav nav = getNav(navs, i);
      Angle<double> boatDir = nav.gpsBearing();
      HorizontalMotion<double> trueWind = estimateTrueWindUsingEstimator(nav);
      Angle<double> twa = calcTwa(trueWind, boatDir)
          + Angle<double>::degrees(360);
      Angle<double> etwa = nav.externalTwa();
      Angle<double> dif = (twa - etwa).normalizedAt0();
      difs[i] = dif;

      if (fabs(dif) < tol) {
        counter++;
      }
    }

    double successrate = double(counter)/count;
    // TODO: I had to lower this threshold slightly after
    // rewriting the NMEA0183 parsing code, because we process the GLL
    // sequence now, which affects the number of sampled navs.
    EXPECT_LE(0.77, successrate);
    EXPECT_LE(getMedianAbsValue(difs).degrees(), 3.0);
  }
}

TEST(TrueWindEstimatorTest, AlinghiGC32Test) {
  // Based on the data found here: https://github.com/jpilet/anemomind/issues/743#issuecomment-229959644
  /*
   *
HERE ARE THE SOURCE MEASURES:
  awa = 315.96 degrees
  aws = 5.69 knots
  gpsBearing = 23.59 degrees
  gpsSpeed = 6.8 knots
  gps-motion-x = 2.72129 knots
  gps-motion-y = 6.23174 knots
NAIVE TRUE WIND AT 2016-06-22T15:27:00
  TWDIR: -100.823
  TWA:   -124.413
  TWS:   4.79461
   */

  Nav nav;
  nav.setAwa(Angle<double>::degrees(315.96));
  nav.setAws(Velocity<double>::knots(5.69));
  nav.setGpsBearing(Angle<double>::degrees(23.59));
  nav.setGpsSpeed(Velocity<double>::knots(6.8));

  double parameters[TrueWindEstimator::NUM_PARAMS];
      TrueWindEstimator::initializeParameters(parameters);
  auto tw = TrueWindEstimator::computeTrueWind
        <double>(parameters, nav);


  auto twa = calcTwa(tw, nav.gpsBearing()).normalizedAt0();
  auto tws = tw.norm();
  auto twdir = (tw.angle() - Angle<double>::degrees(180)).normalizedAt0();

  EXPECT_NEAR(twa.degrees(), -124.413, 0.01);
  EXPECT_NEAR(tws.knots(), 4.79461, 0.01);
  EXPECT_NEAR(twdir.degrees(), -100.823, 0.01);
}
