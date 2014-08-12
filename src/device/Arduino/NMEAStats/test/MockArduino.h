#ifndef _DEVICE_MOCK_ARDUINO_H
#define _DEVICE_MOCK_ARDUINO_H

#include <gmock/gmock.h>
#include <string>

// Allows faking an arduino for testing.
// This object uses global variables, due to Arduino code architecture.
// No more than 1 instance can be constructed at the same time.
class MockArduino {
  public:
    MockArduino();
    ~MockArduino();

    // The following methods are virtual for easier testing.
    virtual long millis() { return _arduinoTimeMs; }
    virtual long micros() { return _arduinoTimeMs * 1000; }
    virtual void delay(long ms) { _arduinoTimeMs += ms; }
    MOCK_METHOD0(screenInit, void());
    MOCK_METHOD1(screenUpdate, void(int a));
    MOCK_METHOD3(screenUpdate, void(int a, int b, int c));

    void setMillis(long ms) { _arduinoTimeMs = ms; }
  private:
    long _arduinoTimeMs;
};

void sendDataToArduino(const std::string& data);

#endif  // _DEVICE_MOCK_ARDUINO_H
