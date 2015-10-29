#ifndef ANEMOBOX_REPLAYDISPATCHER_H
#define ANEMOBOX_REPLAYDISPATCHER_H

#include <device/anemobox/Dispatcher.h>
#include <server/nautical/Nav.h>

namespace sail {

class ReplayDispatcher : public Dispatcher {
 public:
  virtual TimeStamp currentTime() { return _replayTime; }

  void advanceTo(const Nav& nav);

  const char* sourceName() { return "Replay"; }
 private:
  TimeStamp _replayTime;
};

}  // namespace sail

#endif  // ANEMOBOX_REPLAYDISPATCHER_H
