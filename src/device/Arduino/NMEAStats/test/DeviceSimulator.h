#ifndef _NMEA_STATS_DEVICE_SIMULATOR_H
#define _NMEA_STATS_DEVICE_SIMULATOR_H

#include <string>

#include <device/Arduino/NMEAStats/test/MockArduino/SD.h>
#include <device/Arduino/NMEAStats/test/MockArduino/SoftwareSerial.h>
#include <server/common/TimeStamp.h>

// Interface to simulate an Arduino.
//
// screenInit and screenUpdate methods have to be implemented.
//
// This object uses global variables, due to Arduino code architecture.
// No more than 1 instance can be constructed at the same time.
class DeviceSimulator {
 public:
  DeviceSimulator();
  virtual ~DeviceSimulator();
  virtual long millis() { return _arduinoTimeMs; }
  virtual long micros() { return _arduinoTimeMs * 1000; }
  virtual void delay(long ms) { _arduinoTimeMs += ms; }
  virtual void screenInit() { };
  virtual void screenUpdate(int a) { };
  virtual void screenUpdate(int perf, int twdir, int tws) { };

  void setMillis(long ms) { _arduinoTimeMs = ms; }

  // Calls the Arduino 'Setup' method.
  void setup();

  // Send NMEA data through the serial port,
  // sets the time according to NMEA stream,
  // calls the loop() Arduino function until data is consumed.
  void sendData(const std::string& data);

  // Return a pointer to the SD object used by the Arduino.
  MockSD* SD();

  sail::TimeStamp getTimeStamp() const;
 private:
  long _arduinoTimeMs;
  sail::TimeStamp _referenceTime;
  long _referenceMillis;
};

#endif  // _NMEA_STATS_DEVICE_SIMULATOR_H

