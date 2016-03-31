#include <device/anemobox/simulator/SimulateBox.h>

#include <device/Arduino/libraries/TrueWindEstimator/TrueWindEstimator.h>
#include <device/anemobox/DispatcherTrueWindEstimator.h>
#include <device/anemobox/simulator/ReplayDispatcher.h>
#include <fstream>
#include <server/common/Functional.h>
#include <server/common/Span.h>

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
          windMotionFromTwdirAndTws(replay.val<TWDIR>(),
                                    replay.val<TWS>()));
    }
  }
  return true;
}


void replayDispatcherTrueWindEstimator(Dispatcher *src,
                                       ReplayDispatcher2 *replay, 
                                       std::function<void()> cb) {

  int counter = 0;
  replay->subscribe(cb);
  replay->replay(src);
  replay->unsubscribeLast();
}


NavDataset SimulateBox(std::istream& boatDat, const NavDataset &ds) {
  // This code is just a concept. To be unit tested in a subsequent PR.
  *((unsigned int*)nullptr) = 0xDEADBEEF;

  auto replay = new ReplayDispatcher2(
      Duration<double>::seconds(0.1),
      Duration<double>::milliseconds(20));
  DispatcherTrueWindEstimator estimator(replay);
  if (!estimator.loadCalibration(boatDat)) {
    return NavDataset();
  }
  auto srcName = std::string("Simulated ") + estimator.sourceName();
  auto src = ds.dispatcher().get();
  replayDispatcherTrueWindEstimator(src, replay,
                                    [&]() {estimator.compute(srcName);});
  copyPriorities(src, replay);
  replay->setSourcePriority(srcName, replay->sourcePriority(estimator.sourceName()) + 1);
  return NavDataset(std::shared_ptr<Dispatcher>(replay));
}

}  // namespace sail
