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

namespace {
  void listInterestingTimeStamps(Dispatcher *d,
                                 const std::set<DataCode> &triggeringCodes,
                                 std::vector<TimeStamp> *dst) {
    ReplayDispatcher2 replay;
    replay.replay(d, [&](DataCode c, const std::string &src) {
        if (triggeringCodes.count(c) == 1) {
          dst->push_back(replay.currentTime());
        }
    });
  }


  Array<bool> makeTriggerMaskMin(const std::vector<TimeStamp> &ts, 
                              const Duration<double> &period) {
    auto last = ts.size() - 1;
    return sail::map(Spani(0, ts.size()), ts, [&](int i, TimeStamp t) {
        if (i == last) {
          return true;
        } else {
          return period < (ts[i + 1] - t);
        }
      }).toArray();
  }

  Array<bool> makeTriggerMask(const std::vector<TimeStamp> &ts, 
                              const Duration<double> &minPeriod,
                              const Duration<double> &maxPeriod) {

    // First make a minimum triggering mask, that triggers whenever
    // it is at least 'minPeriod' to the next sample
    auto mask = makeTriggerMaskMin(ts, minPeriod);

    assert(mask.size() == ts.size());

    // Then trigger a few more times, so that the sampling is ensured to be dense enough.
    TimeStamp lastTriggered;
    for (int i = 0; i < mask.size(); i++) {
      if (mask[i]) {
        lastTriggered = ts[i];
      } else {
        if (lastTriggered.defined() && (maxPeriod < ts[i] - lastTriggered)) {
          mask[i] = true;
          lastTriggered = ts[i];
        }
      }
    }
    return mask;
  }
}


void replayDispatcherTrueWindEstimator(Dispatcher *src,
                                       ReplayDispatcher2 *replay, 
                                       std::function<void()> cb) {

  std::set<DataCode> triggeringCodes{
    AWA, AWS, GPS_SPEED, GPS_BEARING,
      WAT_SPEED, MAG_HEADING
  };
  
  std::vector<TimeStamp> ts;
  listInterestingTimeStamps(src, triggeringCodes, &ts);
  auto mask = makeTriggerMask(ts, Duration<double>::milliseconds(20),
                              Duration<double>::seconds(1.0));

  int counter = 0;
  replay->replay(src, [&](DataCode c, const std::string &src) {
      if (triggeringCodes.count(c) == 1) {
        if (mask[counter]) {
          cb();
        }
      }
      counter++;
    });
  assert(counter == ts.size());
}


NavDataset SimulateBox(std::istream& boatDat, const NavDataset &ds) {
  // This code is just a concept. To be unit tested in a subsequent PR.
  *((unsigned int*)nullptr) = 0xDEADBEEF;

  auto replay = new ReplayDispatcher2();
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
