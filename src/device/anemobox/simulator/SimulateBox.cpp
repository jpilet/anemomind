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

  class EstimateOnNewValue:
    public Listener<Angle<double>>,
    public Listener<Velocity<double>> {
   public:
     EstimateOnNewValue(DispatcherTrueWindEstimator *estimator,
                        const std::string& srcName,
                        ReplayDispatcher *replayDispatcher)
         : _estimator(estimator), _srcName(srcName), _timer(false),
         _replayDispatcher(replayDispatcher) {

       // This list should match the 'triggeringFields' in estimator.js
       replayDispatcher->get<AWA>()->dispatcher()->subscribe(this);
       replayDispatcher->get<AWS>()->dispatcher()->subscribe(this);
       replayDispatcher->get<GPS_SPEED>()->dispatcher()->subscribe(this);
       replayDispatcher->get<GPS_BEARING>()->dispatcher()->subscribe(this);
       replayDispatcher->get<WAT_SPEED>()->dispatcher()->subscribe(this);
       replayDispatcher->get<MAG_HEADING>()->dispatcher()->subscribe(this);

       // The _onTimeout callback is created as a member to avoid closure
       // creation (and environment capture) at every estimate() call.
       // The methode estimate() is called many time during replay.
       _onTimeout = [&]() {
         _timer = false;
         _estimator->compute(_srcName);
       };
     }

    void estimate() {
      // Inspired by the function 'start' in anemonode/components/estimator.js
      if (!_timer) {
        _timer = true;
        _replayDispatcher->setTimeout(_onTimeout, 20);
      }
    }

    virtual void onNewValue(const ValueDispatcher<Angle<double> > &) {
      estimate();
    }

    virtual void onNewValue(const ValueDispatcher<Velocity<double> > &) {
      estimate();
    }

   private:
    DispatcherTrueWindEstimator* _estimator;
    std::string _srcName;
    bool _timer;
    std::function<void()> _onTimeout;
    ReplayDispatcher *_replayDispatcher;
  };
}



/*
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
*/

NavDataset SimulateBox(const std::string& boatDat, const NavDataset &ds) {
  std::ifstream file(boatDat);
  return SimulateBox(file, ds);
}

NavDataset SimulateBox(std::istream &boatDat, const NavDataset &ds) {
  auto replay = std::make_shared<ReplayDispatcher>();
  DispatcherTrueWindEstimator estimator(replay.get());
  if (!estimator.loadCalibration(boatDat)) {
    return NavDataset();
  }
  auto srcName = std::string("Simulated ") + estimator.sourceName();

  // For now, the UI makes no difference between what has been computed
  // live on the box and what is simulated.
  // Therefore, it is useless to let both go through.
  // We keep only the simulated stuff, except for TWA and TWS, because
  // they can go in 'externalTw[sa]'.
  NavDataset src = ds
    .stripChannel(TWDIR)
    .stripChannel(TARGET_VMG);

  EstimateOnNewValue listener(&estimator, srcName, replay.get());

  replay.get()->replay(src.dispatcher().get());

  replay->setSourcePriority(srcName, replay->sourcePriority(estimator.sourceName()) + 1);
  return NavDataset(std::static_pointer_cast<Dispatcher>(replay));
}

}  // namespace sail
