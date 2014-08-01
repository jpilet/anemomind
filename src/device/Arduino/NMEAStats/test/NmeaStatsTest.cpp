#include <gtest/gtest.h>

#include "../NMEAStats.ino"

#include <iostream>

MockSD SD;
MockSerial Serial;
long arduinoTimeMs = 1234;

long millis() {
  return arduinoTimeMs;
}

long micros() {
  return millis() * 1000;
}

void delay(long ms) {
  arduinoTimeMs += ms;
}

// Mock Screen.cpp
void screenInit() { }
void screenUpdate(int) { }

int expectedPerf = -1;
int expectedTwdir = -1;
int expectedTws = -1;
bool gotScreenUpdate = false;
void screenUpdate(int perf, int twdir, int tws) {
  EXPECT_EQ(expectedPerf, perf);
  EXPECT_EQ(expectedTwdir, twdir);
  EXPECT_EQ(expectedTws, tws);
  gotScreenUpdate = true;
}
void expectDisplay(int perf, int twdir, int tws) {
  expectedPerf = perf;
  expectedTwdir = twdir;
  expectedTws = tws;
  gotScreenUpdate = false;
}

TEST(DeviceTest, LogTest) {
  const char data[] =
    "IIRMC,164508,A,4629.737,N,00639.791,E,03.6,188,200808,,,A*48\n"
    "IIVHW,,,192,M,03.4,N,,*69\n"
    "IIVWR,030,L,07.8,N,,,,*73\n"
    "IIDPT,054.6,-1.0,*47\n"
    "IIGLL,4629.736,N,00639.790,E,164510,A,A*5E\n"
    "IIHDG,192,,,,*5D\n"
    "IIMTW,+20.5,C*3F\n"
    "IIMWV,330,R,07.8,N,A*1C\n"
    "IIMWV,315,T,04.9,N,A*1F\n"
    "IIRMB,A,0.11,R,,YVOI,,,,,015.92,242,02.4,V,A*73\n"
    "IIRMC,164510,A,4629.736,N,00639.790,E,03.6,197,200808,,,A*4F\n"
    "IIVHW,,,195,M,03.4,N,,*6E\n"
    "IIVWR,028,L,07.7,N,,,,*75\n"
    "IIDPT,062.0,-1.0,*44\n"
    "IIGLL,4629.736,N,00639.790,E,164510,A,A*5E\n";
  Serial.setData(data);

  setup();

  expectDisplay(0, 0, 0);

  for (int i = 0; i < 10000; ++i) {
    loop();
  }
  EXPECT_TRUE(displaySpeedRatio
  EXPECT_EQ(std::string(data), Serial.output());
}
