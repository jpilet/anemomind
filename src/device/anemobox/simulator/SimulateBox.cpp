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

  class EstimateOnNewValue;

  template<typename T>
  class OnValue : public Listener<T> {
    public:
      OnValue<T>(EstimateOnNewValue* estimator) : _estimator(estimator) { }
      void onNewValue(const ValueDispatcher<T>&) override;
    private:
      EstimateOnNewValue* _estimator;
  };

  class EstimateOnNewValue {
   public:
     EstimateOnNewValue(DispatcherTrueWindEstimator *estimator,
                        const std::string& srcName,
                        ReplayDispatcher *replayDispatcher)
         : _estimator(estimator), _srcName(srcName), _timer(false),
         _replayDispatcher(replayDispatcher),
          awaListener(this),
          awsListener(this),
          gpsSpeedListener(this),
          gpsBearingListener(this),
          watSpeedListener(this),
          magHeadingListener(this) {
       // This list should match the 'triggeringFields' in estimator.js
       replayDispatcher->get<AWA>()->dispatcher()->subscribe(&awaListener);
       replayDispatcher->get<AWS>()->dispatcher()->subscribe(&awsListener);
       replayDispatcher->get<GPS_SPEED>()->dispatcher()->subscribe(&gpsSpeedListener);
       replayDispatcher->get<GPS_BEARING>()->dispatcher()->subscribe(&gpsBearingListener);
       replayDispatcher->get<WAT_SPEED>()->dispatcher()->subscribe(&watSpeedListener);
       replayDispatcher->get<MAG_HEADING>()->dispatcher()->subscribe(&magHeadingListener);

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

   private:
    DispatcherTrueWindEstimator* _estimator;
    std::string _srcName;
    bool _timer;
    std::function<void()> _onTimeout;
    ReplayDispatcher *_replayDispatcher;

    // A Listener can listen only to one channel at a time, so we need one
    // listener per channel.
    OnValue<Angle<>> awaListener;
    OnValue<Velocity<>> awsListener;
    OnValue<Velocity<>> gpsSpeedListener;
    OnValue<Angle<>> gpsBearingListener;
    OnValue<Velocity<>> watSpeedListener;
    OnValue<Angle<>> magHeadingListener;
  };


template<typename T>
void OnValue<T>::onNewValue(const ValueDispatcher<T>&) {
  _estimator->estimate();
}

}  // namespace

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

NavDataset SimulateBox(std::istream &boatDat, const NavDataset &src) {
  auto replay = std::make_shared<ReplayDispatcher>();
  DispatcherTrueWindEstimator estimator(replay.get());
  if (!estimator.loadCalibration(boatDat)) {
    return NavDataset();
  }
  auto srcName = std::string("Simulated ") + estimator.sourceName();

  EstimateOnNewValue listener(&estimator, srcName, replay.get());

  copyPriorities(src.dispatcher().get(), replay.get());

  replay.get()->replay(src.dispatcher().get());

  replay->setSourcePriority(srcName, replay->sourcePriority(estimator.sourceName()) + 1);

  NavDataset result(std::static_pointer_cast<Dispatcher>(replay));

  for (DataCode code : allDataCodes()) {
    std::shared_ptr<DispatchData> active(src.activeChannelOrNull(code));
    if (active) {
      result.preferSource(code, active->source());
    }
  }

  return result;
}

}  // namespace sail
