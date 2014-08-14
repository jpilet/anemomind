#include <device/Arduino/NMEAStats/test/MockArduino.h>

#include <device/Arduino/NMEAStats/test/MockArduino/SD.h>
#include <device/Arduino/NMEAStats/test/MockArduino/SPI.h>
#include <device/Arduino/NMEAStats/test/MockArduino/SoftwareSerial.h>
#include <device/Arduino/libraries/NmeaParser/NmeaParser.h>
#include <server/common/TimeStamp.h>
#include <server/common/logging.h>
#include <server/nautical/NavNmea.h>

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
  Serial.clear();
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

  NmeaParser parser;
  bool timeInitialized = false;
  sail::TimeStamp referenceTime;
  long referenceMillis;

  for (auto c : data) {
    if (parser.processByte(c) == NmeaParser::NMEA_TIME_POS) {
      if (!timeInitialized) {
        timeInitialized = true;
        referenceMillis = millis();
        referenceTime = sail::getTime(parser);
      } else {
        sail::TimeStamp now = sail::getTime(parser);
        long minMillis((now - referenceTime).milliseconds() + referenceMillis);
        long maxMillis(minMillis + 1000);

        if (millis() < minMillis) {
          fakeArduino->setMillis(minMillis);
        } else if (millis() > maxMillis) {
          fakeArduino->setMillis(maxMillis);
        }
      }
    } else {
      // At 4800 bps, it takes 1000/480 milliseconds to transmit
      // 8 bits with 1 start bit and 1 stop bit.
      fakeArduino->delay(1000 / 480);
    }

    std::string buffer;
    buffer += c;
    EXPECT_EQ(0, Serial.available());
    Serial.setData(buffer);
    loop();
    EXPECT_EQ(0, Serial.available());
  }
}

