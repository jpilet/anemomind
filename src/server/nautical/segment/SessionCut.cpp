/*
 * SessionCut.cpp
 *
 *  Created on: May 20, 2016
 *      Author: jonas
 */

#include <server/nautical/segment/SessionCut.h>
#include <server/common/AbstractArray.h>
#include <server/common/ArrayBuilder.h>
#include <server/common/logging.h>
#include <server/math/hmm/StateAssign.h>

namespace sail {
namespace SessionCut {

Settings::Settings() :
  cuttingThreshold(Duration<double>::seconds(5.0)),
  regularization(0.0) {}

namespace {

  enum {
    TURNED_OFF = 0,
    TURNED_ON = 1
  };

/*
 * At duration x = 0, the cost is -1 (that is a reward)
 * At duration x = cuttingThreshold, the cost is 0.
 * Those to points belong to a straight line that defines all costs.
 * So, for instance, at x = 2*cuttingThreshold, the cost will be 1.
 */
double computeDurationCost(Duration<double> x, const Settings &s) {
  return double(x/s.cuttingThreshold) - 1.0;
}

void allGoodOrDie(const AbstractArray<TimeStamp> &timeStamps) {
  {
    for (int i = 0; i < timeStamps.size(); i++) {
      if (!timeStamps[i].defined()) {
        LOG(FATAL) << "Undefined time stamp at " << i;
      }
    }
  }{
    int n = timeStamps.size() - 1;
    for (int i = 0; i < n; i++) {
      if (timeStamps[i] > timeStamps[i+1]) {
        LOG(FATAL) << "Time stamps not ordered at " << i;
      }
    }
  }
}

class SessionStateAssign : public StateAssign {
 public:

  // TODO: Adding extra costs based on what a user might have defined
  // as being a session, or not.

  SessionStateAssign(const AbstractArray<TimeStamp> &timeStamps,
      const Settings &settings) :
        _timeStamps(timeStamps),
        _settings(settings),
        _preceding({TURNED_OFF, TURNED_ON}) {}

  double getStateCost(int stateIndex, int timeIndex) override {
    if (stateIndex == TURNED_ON) {
      return computeDurationCost(
          _timeStamps[timeIndex+1] - _timeStamps[timeIndex],
          _settings);
    }
    return 0.0;
  }

  double getTransitionCost(int fromStateIndex,
      int toStateIndex, int fromTimeIndex) override {
    return std::abs(fromStateIndex - toStateIndex)*_settings.regularization;
  }

  int getStateCount() override {
    return 2;
  }

  int getLength() override {
    return _timeStamps.size() - 1;
  }

  Arrayi getPrecedingStates(int stateIndex, int timeIndex) override {
    return _preceding;
  }
 private:
  Arrayi _preceding;
  const AbstractArray<TimeStamp> &_timeStamps;
  const Settings &_settings;
};

Array<Span<TimeStamp> > listSpans(
    const Arrayi &states,
    const AbstractArray<TimeStamp> &times) {
  int lastState = TURNED_OFF;
  TimeStamp from;
  ArrayBuilder<Span<TimeStamp> > dst;
  for (int i = 0; i < states.size(); i++) {
    auto state = states[i];
    if (state != lastState) {
      if (state == TURNED_ON) {
        from = times[i];
      } else {
        dst.add(Span<TimeStamp>(from, times[i]));
      }
    }
    lastState = state;
  }
  if (lastState == TURNED_ON) {
    dst.add(Span<TimeStamp>(from, times.last()));
  }
  return dst.get();
}

}


Array<Span<TimeStamp> > cutSessions(
    const AbstractArray<TimeStamp> &timeStamps, const Settings &settings) {
  allGoodOrDie(timeStamps);
  auto states = SessionStateAssign(timeStamps, settings).solve();
  return listSpans(states, timeStamps);
}




}
}
