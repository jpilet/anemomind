#ifndef ANEMOBOX_FAKE_CLOCK_DISPATCHER_H
#define ANEMOBOX_FAKE_CLOCK_DISPATCHER_H

#include <device/anemobox/Dispatcher.h>

namespace sail {

class FakeClockDispatcher : public Dispatcher {
 public:
  FakeClockDispatcher() : _currentTime(TimeStamp::now()) { }
  virtual TimeStamp currentTime() { return _currentTime; }

  void setTime(TimeStamp time) { _currentTime = time; }
  void advance(Duration<> delta) { _currentTime = _currentTime + delta; }
 private:
  TimeStamp _currentTime;
};

}  // namespace sail

#endif  // ANEMOBOX_FAKE_CLOCK_DISPATCHER_H
