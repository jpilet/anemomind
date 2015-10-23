#include <device/anemobox/simulator/SimulateBox.h>

#include <device/anemobox/DispatcherTrueWindEstimator.h>
#include <device/anemobox/simulator/ReplayDispatcher.h>

#include <fstream>

namespace sail {

namespace {

template <DataCode Code>
bool isFresh(const Dispatcher& dispatcher) {
  return dispatcher.get<Code>()->isFresh(Duration<>::seconds(.1));
}

} // namespace

bool SimulateBox(const std::string& boatDat, Array<Nav> *navs) {
  std::ifstream file(boatDat, std::ios::in | std::ios::binary);

  if (!file.good()) {
    return false;
  }

  return SimulateBox(file, navs);
}

bool SimulateBox(std::istream& boatDat, Array<Nav> *navs) {
  ReplayDispatcher replay;
  DispatcherTrueWindEstimator estimator(&replay);

  if (!estimator.loadCalibration(boatDat)) {
    return false;
  }

  for (Nav& nav : *navs) {
    replay.advanceTo(nav);
    estimator.compute();

    // save true wind from dispatcher to NAV
    if (isFresh<TARGET_VMG>(replay)) {
      nav.setDeviceTargetVmg(replay.val<TARGET_VMG>());
    }
    if (isFresh<VMG>(replay)) { nav.setDeviceVmg(replay.val<VMG>()); }
    if (isFresh<TWS>(replay)) { nav.setDeviceTws(replay.val<TWS>()); }
    if (isFresh<TWA>(replay)) { nav.setDeviceTwa(replay.val<TWA>()); }
    if (isFresh<TWS>(replay)) { nav.setDeviceTws(replay.val<TWS>()); }
    if (isFresh<TWDIR>(replay)) {
      nav.setDeviceTwdir(replay.val<TWDIR>());
      nav.setTrueWindOverGround(
          HorizontalMotion<double>::polar(
              replay.val<TWS>(), replay.val<TWDIR>() - Angle<>::degrees(180)));
    }
  }
  return true;
}

}  // namespace sail
