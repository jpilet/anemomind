#include <device/Arduino/libraries/TrueWindEstimator/TrueWindEstimator.h>

#include <device/Arduino/libraries/TrueWindEstimator/InstrumentFilter.h>
#include <gtest/gtest.h>
#include <server/common/Env.h>
#include <server/common/logging.h>
#include <server/nautical/NavNmea.h>
#include <sstream>
#include <string>

using std::string;
using namespace sail;

TEST(TrueWindEstimatorTest, SmokeTest) {
  Array<Nav> navs = loadNavsFromNmea(
      string(Env::SOURCE_DIR) + string("/datasets/tinylog.txt"),
      Nav::Id("B0A10000")).navs();

  CHECK_LT(0, navs.size());

  double parameters[TrueWindEstimator::NUM_PARAMS];
  TrueWindEstimator::initializeParameters(parameters);

  auto trueWind = TrueWindEstimator::computeTrueWind
    <double,InstrumentFilter<double> >(parameters, makeFilter(navs));

  LOG(INFO) << trueWind[0].knots() << ", " << trueWind[1].knots();
}

TEST(TrueWindEstimatorTest, ManuallyCheckedDataTest) {
  // Sailing upwind. True wind: ~222, at about 16.2 knots.
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
  ParsedNavs navs = loadNavsFromNmea(stream, Nav::debuggingBoatId());

  EXPECT_TRUE(navs.navs().hasData());
  EXPECT_EQ(1, navs.navs().size());

  double parameters[TrueWindEstimator::NUM_PARAMS];
  TrueWindEstimator::initializeParameters(parameters);

  auto trueWind = TrueWindEstimator::computeTrueWind
    <double,InstrumentFilter<double> >(parameters, makeFilter(navs.navs()));

  EXPECT_NEAR(222, trueWind.angle().degrees() + 360, 5);
  EXPECT_NEAR(16, trueWind.norm().knots(), 1);
}
