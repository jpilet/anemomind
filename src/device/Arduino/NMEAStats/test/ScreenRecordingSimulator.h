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

  bool screenAt(TimeStamp time, ScreenInfo *info);

  void sort();

  const std::vector<ScreenInfo> screenInfo() const { return _screenInfo; }

  void prepare(const std::string& boatDatFilename,
               const std::string& polarDatFilename);

  void simulate(std::string filename);

  void simulate(const std::vector<std::string>& nmeaFiles,
                const char* boatDatFilename,
                const char* polarDatFilename);
 private:
  std::vector<ScreenInfo> _screenInfo;
  bool _sorted;
};
      
}  // namespace sail

#endif  // DEVICE_SCREEN_RECORDING_SIMULATOR_H
