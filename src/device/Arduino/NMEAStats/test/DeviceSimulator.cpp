#include <cmath>
#include <device/Arduino/NMEAStats/test/DeviceSimulator.h>

#include <device/Arduino/NMEAStats/test/MockArduino/SD.h>
#include <device/Arduino/NMEAStats/test/MockArduino/SoftwareSerial.h>
MockSD SD;
MockSerial Serial;

// Declare the Arduino functions defined in the .ino file.
extern void loop();
extern void setup();

// Include Arduino code.
#include "../NMEAStats.ino"

#include <device/Arduino/NMEAStats/test/MockArduino/SPI.h>
#include <device/Arduino/libraries/NmeaParser/NmeaParser.h>
#include <server/common/logging.h>
#include <server/nautical/logs/LogLoader.h>


DeviceSimulator *fakeArduino = 0;

DeviceSimulator::DeviceSimulator() : _arduinoTimeMs(1234), _referenceMillis(0) {
  CHECK(fakeArduino == 0);
  fakeArduino = this;
}

DeviceSimulator::~DeviceSimulator() {
  ::SD.closeAll();
  Serial.clear();
  fakeArduino = 0;
}

void DeviceSimulator::setup() {
  ::setup();
}

MockSD* DeviceSimulator::SD() { return &::SD; }

// Declared in MockArduino/Arduino.h
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

sail::TimeStamp DeviceSimulator::getTimeStamp() const {
  return _referenceTime + Duration<>::milliseconds(
      _arduinoTimeMs - _referenceMillis);
}

void DeviceSimulator::sendData(const std::string& data) {

  NmeaParser parser;
  bool timeInitialized = false;

  for (auto c : data) {
    if (NmeaParser::isCycleMark(parser.processByte(c))) {
      if (!timeInitialized) {
        timeInitialized = true;
        _referenceMillis = millis();
        _referenceTime = sail::getTime(parser);
      } else {
        sail::TimeStamp now = sail::getTime(parser);
        long minMillis((now - _referenceTime).milliseconds() + _referenceMillis);
        long maxMillis(minMillis + 1000);

        if (millis() < minMillis) {
          setMillis(minMillis);
        } else if (millis() > maxMillis) {
          setMillis(maxMillis);
        }
      }
    } else {
      // At 4800 bps, it takes 1000/480 milliseconds to transmit
      // 8 bits with 1 start bit and 1 stop bit.
      delay(1000 / 480);
    }

    std::string buffer;
    buffer += c;
    CHECK(Serial.available() == 0);
    Serial.setData(buffer);
    loop();
    CHECK(Serial.available() == 0);
  }
}

bool DeviceSimulator::polarTableLoadedOrDisabled() const {
#ifdef VMG_TARGET_SPEED
  return true;
#else
  return !polarSpeedTable.empty();
#endif
}

bool DeviceSimulator::calibrationFileLoaded() const {
  return calibrationLoaded;
}

bool DeviceSimulator::compiledWithVMGTargetSpeed() const {
#ifdef VMG_TARGET_SPEED
  return true;
#else
  return false;
#endif
}
