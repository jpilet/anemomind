#include <device/anemobox/simulator/SimulateBox.h>

#include <device/Arduino/libraries/TrueWindEstimator/TrueWindEstimator.h>
#include <device/anemobox/DispatcherTrueWindEstimator.h>
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
}



// Inspired by the function 'start' in anemonode/components/estimator.js
void generateComputeCallbacks(Dispatcher *src,
    ReplayDispatcher *replayDispatcher,
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
#define SUBSCRIBE_CALLBACK(TYPE) \
  EventListener TYPE##listener(update); \
  replayDispatcher->get<TYPE>()->dispatcher()->subscribe(&TYPE##listener);

  SUBSCRIBE_CALLBACK(AWA);
  SUBSCRIBE_CALLBACK(AWS);
  SUBSCRIBE_CALLBACK(GPS_SPEED);
  SUBSCRIBE_CALLBACK(GPS_BEARING);
  SUBSCRIBE_CALLBACK(WAT_SPEED);
  SUBSCRIBE_CALLBACK(MAG_HEADING);
#undef SUBSCRIBE_CALLBACK

  replayDispatcher->replay(src);
}

NavDataset SimulateBox(const std::string& boatDat, const NavDataset &ds) {
  std::ifstream file(boatDat);
  return SimulateBox(file, ds);
}

NavDataset SimulateBox(std::istream &boatDat, const NavDataset &ds) {
  auto replay = new ReplayDispatcher();
  DispatcherTrueWindEstimator estimator(replay);
  if (!estimator.loadCalibration(boatDat)) {
    LOG(ERROR) << "Failed to load calibration";
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
