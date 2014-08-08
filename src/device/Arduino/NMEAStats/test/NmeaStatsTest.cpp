#include <gtest/gtest.h>

#include <device/Arduino/NMEAStats/test/MockArduino.h>

// Include Arduino code.
#include "../NMEAStats.ino"

#include <iostream>


TEST(DeviceTest, LogTest) {
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

  MockArduino arduino;

  EXPECT_CALL(arduino, screenUpdate(testing::_)).Times(testing::AtLeast(1));
  EXPECT_CALL(arduino, screenInit());

  setup();

  // The first call contains only junk: no wind data has been sent yet.
  EXPECT_CALL(arduino, screenUpdate(testing::_, testing::_, testing::_)).Times(1);

  // The second call contains something meaningful.
  // The speed ratio is 0 because no target speed table has been loaded.
  EXPECT_CALL(arduino, screenUpdate(0, 315, 5)).Times(1);

  sendDataToArduino(data);

  // The code should log all input data.
  EXPECT_EQ(std::string(data), SD.getWrittenFile("nmea0000.txt"));
}
