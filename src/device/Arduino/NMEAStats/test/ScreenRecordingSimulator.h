#ifndef DEVICE_SCREEN_RECORDING_SIMULATOR_H
#define DEVICE_SCREEN_RECORDING_SIMULATOR_H

#include <device/Arduino/NMEAStats/test/DeviceSimulator.h>
#include <server/common/TimeStamp.h>
#include <vector>

namespace sail {

struct ScreenInfo {
  TimeStamp time;
  int perf;
  int twdir;
  int tws;

  bool operator <(const ScreenInfo& other) const {
    return time < other.time;
  }
};

class ScreenRecordingSimulator : public DeviceSimulator {
 public:
  ScreenRecordingSimulator() : _sorted(false) { }

  virtual void screenUpdate(int perf, int twdir, int tws);

  ScreenInfo screenAt(TimeStamp time);

  void sort();

  const std::vector<ScreenInfo> screenInfo() const { return _screenInfo; }

  void simulate(const std::vector<std::string>& nmeaFiles,
                const char* boatDatFilename,
                const char* polarDatFilename);
 private:
  std::vector<ScreenInfo> _screenInfo;
  bool _sorted;
};
      
}  // namespace sail

#endif  // DEVICE_SCREEN_RECORDING_SIMULATOR_H
