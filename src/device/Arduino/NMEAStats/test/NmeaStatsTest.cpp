#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <device/Arduino/NMEAStats/test/DeviceSimulator.h>
#include <server/common/string.h>
#include <server/common/Env.h>
#include <server/common/PathBuilder.h>

#define ON_SERVER
#include <device/Arduino/libraries/TargetSpeed/PolarSpeedTable.h>
#undef ON_SERVER

#include <iostream>


namespace {

// Fakes an arduino for testing.
class MockArduino : public DeviceSimulator {
  public:
    // Hook virtual methods with gmock.
    MOCK_METHOD0(screenInit, void());
    MOCK_METHOD1(screenUpdate, void(int a));
    MOCK_METHOD3(screenUpdate, void(int a, int b, int c));
};

const char data[] =
  "$IIRMC,164508,A,4629.737,N,00639.791,E,03.6,188,200808,,,A*48\n"
  "$IIVHW,,,192,M,03.4,N,,*69\n"
  "$IIVWR,030,L,07.8,N,,,,*73\n"
  "$IIDPT,054.6,-1.0,*47\n"
  "$IIGLL,4629.736,N,00639.790,E,164510,A,A*5E\n"
  "$IIHDG,192,,,,*5D\n"
  "$IIMTW,+20.5,C*3F\n"
  "$IIMWV,330,R,07.8,N,A*1C\n"
  "$IIMWV,315,T,04.9,N,A*1F\n"
  "$IIRMB,A,0.11,R,,YVOI,,,,,015.92,242,02.4,V,A*73\n"
  "$IIRMC,164510,A,4629.736,N,00639.790,E,03.6,197,200808,,,A*4F\n"
  "$IIVHW,,,195,M,03.4,N,,*6E\n"
  "$IIVWR,028,L,07.7,N,,,,*75\n"
  "$IIDPT,062.0,-1.0,*44\n"
  "$IIGLL,4629.736,N,00639.790,E,164510,A,A*5E\n";
}

TEST(DeviceTest, LogTest) {
  MockArduino arduino;

  EXPECT_CALL(arduino, screenUpdate(testing::_)).Times(testing::AtLeast(1));
  EXPECT_CALL(arduino, screenInit());

  arduino.setup();

  // The first call contains only junk: no wind data has been sent yet.
  EXPECT_CALL(arduino, screenUpdate(testing::_, testing::_, testing::_)).Times(1);

  // The second call contains something meaningful.
  // The speed ratio is 0 because no target speed table has been loaded.
  EXPECT_CALL(arduino, screenUpdate(0, (192 + 315) % 360, 5)).Times(1);

  arduino.sendData(data);

  // The code should log all input data.
  EXPECT_EQ(std::string(data), arduino.SD()->getWrittenFile("nmea0000.txt"));
}

namespace {
  using namespace sail;

  class SpeedLookUp {
   public:
    Velocity<double> operator() (Velocity<double> tws, Angle<double> twa) {
      return 0.8*tws*sin(0.5*twa.radians());
    }
  };

  std::string makeFakePolarFile() {
    Velocity<double> twsStep = Velocity<double>::knots(1.0);
    int twsCount = 30;
    int twaCount = 12;
    std::stringstream ss;
    EXPECT_TRUE(PolarSpeedTable::build(twsStep,
              twsCount, twaCount, SpeedLookUp(), &ss));
    return ss.str();
  }
}

TEST(DeviceTest, CalibratedTest) {
  using namespace sail;

  MockArduino arduino;

  EXPECT_CALL(arduino, screenUpdate(testing::_)).Times(testing::AtLeast(1));
  EXPECT_CALL(arduino, screenInit());

  arduino.SD()->setReadableFile("boat.dat",
      readFileToString(std::string(Env::SOURCE_DIR) +
                       std::string("/src/device/Arduino/NMEAStats/test/boat.dat")));
  arduino.SD()->setReadableFile("polar.dat", makeFakePolarFile());

  arduino.setup();

#ifdef VMG_TARGET_SPEED
  // The first call contains only junk: no wind data has been sent yet.
  EXPECT_CALL(arduino, screenUpdate(testing::_, testing::_, testing::_)).Times(1);

  // The second call contains something meaningful.
  EXPECT_CALL(arduino, screenUpdate(65, 132, 5)).Times(1); // <-- are these values correct? They don't seem consistent with those of LogTest
#else
  // TODO: setup a proper test for the polar target speed.
  EXPECT_CALL(arduino, screenUpdate(testing::_, testing::_, testing::_)).Times(1);
  EXPECT_CALL(arduino, screenUpdate(42, 147, 5)).Times(1);
#endif

  arduino.sendData(data);

  // The code should log all input data.
  EXPECT_EQ(std::string(data), arduino.SD()->getWrittenFile("nmea0000.txt"));
}
