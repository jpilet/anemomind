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
    <double,ServerFilter>(parameters, makeFilter(navs));

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
    <double, ServerFilter>(parameters, makeFilter(navs.navs()));

  EXPECT_NEAR(222, trueWind.angle().degrees() + 360, 5);
  EXPECT_NEAR(16, trueWind.norm().knots(), 1);
}

/*namespace {
  class NavFilter {
   public:
    typedef double type;

    Angle<double> awa() const { return _nav.awa(); }
    Velocity<double> aws() const { return _nav.aws(); }
    Angle<double> magHdg() const { return _nav.magHdg(); }
    Velocity<double> watSpeed() const { return _nav.watSpeed(); }
    Velocity<double> gpsSpeed() const { return _nav.gpsSpeed(); }
    Angle<double> gpsBearing() const { return _nav.gps; }

    HorizontalMotion<T> gpsMotion() const { return _gps._motion; }

    NavFilter(const Nav &nav) : _nav(nav) {}
   private:
    Nav _nav;
  };
}*/

TEST(TrueWindEstimatorTest, TWACompare) {
  Array<Nav> navs = loadNavsFromNmea(
      string(Env::SOURCE_DIR) + string("/datasets/Irene/2007/regate_1_dec_07/IreneLog.txt"),
      Nav::debuggingBoatId()).navs();
  EXPECT_LE(5000, navs.size());
  EXPECT_LE(navs.size(), 7000);

  double parameters[TrueWindEstimator::NUM_PARAMS];
  TrueWindEstimator::initializeParameters(parameters);




  int count = navs.size();
  Angle<double> tol = Angle<double>::degrees(5.0);
  int counter = 0;
  for (int i = 0; i < count; i++) {
    Nav nav = navs[i];
    Angle<double> boatDir = nav.magHdg();
    std::cout << "i = " << i << std::endl;
    /*auto trueWind = TrueWindEstimator::computeTrueWind
      <double>(parameters, navs[i]);*/

    HorizontalMotion<double> boatMotion = nav.gpsMotion();
    HorizontalMotion<double> apparentWind = HorizontalMotion<double>::polar(nav.aws(),
        nav.awa()
          + Angle<double>::radians(M_PI) // We measure the angle to where the wind is coming from
          + boatDir);                    // The AWA angle is relative to the boat heading


    HorizontalMotion<double> trueWind = apparentWind + boatMotion;


    Angle<double> twa = calcTWA(trueWind, boatDir);
    Angle<double> etwa = navs[i].externalTwa();
    Angle<double> dif = (twa - etwa).normalizedAt0();
    if (fabs(dif) < tol) {
      counter++;
    }
  }
  std::cout << "Counter   = " << counter << std::endl;
  std::cout << "Nav count = " << navs.size() << std::endl;
}
