#include <device/Arduino/NMEAStats/test/MockArduino.h>

#include <device/Arduino/NMEAStats/test/MockArduino/SD.h>
#include <device/Arduino/NMEAStats/test/MockArduino/SPI.h>
#include <device/Arduino/NMEAStats/test/MockArduino/SoftwareSerial.h>
#include <server/common/logging.h>

// Declare the Arduino functions defined in the .ino file.
extern void loop();
extern void setup();

MockSD SD;
MockSerial Serial;
MockArduino *fakeArduino = 0;

MockArduino::MockArduino() : _arduinoTimeMs(1234) {
  CHECK(fakeArduino == 0);
  fakeArduino = this;
}

MockArduino::~MockArduino() {
  SD.closeAll();
  fakeArduino = 0;
}

long millis() {
  return fakeArduino->millis();
}

long micros() {
  return fakeArduino->micros();
}

void delay(long ms) {
  fakeArduino->delay(ms);
}

// Mock Screen.cpp
void screenInit() { fakeArduino->screenInit(); }
void screenUpdate(int a) { fakeArduino->screenUpdate(a); }
void screenUpdate(int a, int b, int c) { fakeArduino->screenUpdate(a, b, c); }

void sendDataToArduino(const std::string& data) {
  Serial.setData(data);
  for (int n=0; Serial.available() && n < 10000; ++n) {
    loop();
  }
  EXPECT_EQ(0, Serial.available());
}

