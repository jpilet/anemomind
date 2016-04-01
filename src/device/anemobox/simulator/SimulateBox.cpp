#include <device/anemobox/simulator/SimulateBox.h>

#include <device/Arduino/libraries/TrueWindEstimator/TrueWindEstimator.h>
#include <device/anemobox/DispatcherTrueWindEstimator.h>
#include <device/anemobox/simulator/ReplayDispatcher.h>
#include <fstream>
#include <server/common/Functional.h>
#include <server/common/Span.h>
#include <server/common/logging.h>

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

  class EventListener:
    public Listener<Angle<double>>,
    public Listener<Velocity<double>>,
    public Listener<Length<double>>,
    public Listener<GeographicPosition<double>>,
    public Listener<TimeStamp>,
    public Listener<AbsoluteOrientation> {
   public:
    EventListener(const std::function<void()> &cb) : _cb(cb) {}

    void onNewValue(const ValueDispatcher<Angle<double> > &dispatcher) {
      _cb();
    }

    void onNewValue(const ValueDispatcher<Velocity<double> > &dispatcher) {
      _cb();
    }

    void onNewValue(const ValueDispatcher<Length<double> > &dispatcher) {
      _cb();
    }

    void onNewValue(const ValueDispatcher<GeographicPosition<double> > &dispatcher) {
      _cb();
    }

    void onNewValue(const ValueDispatcher<TimeStamp> &dispatcher) {
      _cb();
    }

    void onNewValue(const ValueDispatcher<AbsoluteOrientation> &dispatcher) {
      _cb();
    }

   private:
    EventListener(const EventListener &other) = delete;

    std::function<void()> _cb;
  };


  class AllocAndSubscribeVisitor {
  public:
    AllocAndSubscribeVisitor(Dispatcher *dst,
        const std::set<DataCode> &codes,
        EventListener *l) :
          _dst(dst), _codes(codes),
          _l(l) {}

    template <DataCode Code, typename T>
    void visit(const char *shortName, const std::string &sourceName,
      const std::shared_ptr<DispatchData> &raw,
      const TimedSampleCollection<T> &coll) {
      _dst->get<Code>()->dispatcher()->subscribe(_l);
    }
  private:
    Dispatcher *_dst;
    const std::set<DataCode> &_codes;
    EventListener *_l;
  };


  // Please keep a reference to the return
  // value of this function as long as you want to listen.
  std::shared_ptr<EventListener> subscribeToData(
      Dispatcher *src,
      Dispatcher *dst,
      const std::set<DataCode> &codes,
      std::function<void()> cb) {

    auto l = std::make_shared<EventListener>(cb);

    AllocAndSubscribeVisitor v(dst, codes, l.get());
    visitDispatcherChannels(src, &v);

    return l;
  }


}



// Inspired by the function 'start' in anemonode/components/estimator.js
void generateComputeCallbacks(Dispatcher *src,
    ReplayDispatcher2 *replayDispatcher,
    std::function<void()> cb) {

  bool timer = false;
  auto update = [&]() {
    if (!timer) {
      timer = true;
      replayDispatcher->setTimeout([&]() {
        timer = false;
        cb();
      }, 20);
    }
  };

  TimeStamp offset;

  // Because a listener can only listen to one thing at a time,
  // (see how the 'listentingTo_' is used in Listener<T>::listen)
  // we need one listener per thing we want to listen to.
#define SUBSCRIBE_CALLBACK(TYPE, CB) \
  EventListener TYPE##listener(CB); \
  replayDispatcher->get<TYPE>()->dispatcher()->subscribe(&TYPE##listener);

  SUBSCRIBE_CALLBACK(AWA, update);
  SUBSCRIBE_CALLBACK(AWS, update);
  SUBSCRIBE_CALLBACK(GPS_SPEED, update);
  SUBSCRIBE_CALLBACK(GPS_BEARING, update);
  SUBSCRIBE_CALLBACK(WAT_SPEED, update);
  SUBSCRIBE_CALLBACK(MAG_HEADING, update);
#undef SUBSCRIBE_CALLBACK

  replayDispatcher->replay(src);
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

  generateComputeCallbacks(src, replay, [&]() {
    estimator.compute(srcName);
  });

  copyPriorities(src, replay);
  replay->setSourcePriority(srcName, replay->sourcePriority(estimator.sourceName()) + 1);
  return NavDataset(std::shared_ptr<Dispatcher>(replay));
}

}  // namespace sail
