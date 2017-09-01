/* julien@anemomind.com, 4.2015
 *
 * Nodejs interface to Dispatcher, Nmea0183Source, etc..
 *
 * See README.md for usage information.
 */
#include <node.h>
#include <nan.h>

#include <device/anemobox/anemonode/src/JsDispatcher.h>
#include <device/anemobox/anemonode/src/JsNmea0183Source.h>
#include <device/anemobox/anemonode/src/JsNmea2000Source.h>
#include <device/anemobox/anemonode/src/JsEstimator.h>
#include <device/anemobox/anemonode/src/JsLogger.h>
#include <device/anemobox/anemonode/src/anemonode.h>
#include <server/common/TimeStamp.h>

#include <iostream>
#include <vector>

#include <sys/time.h>

using namespace sail;
using namespace v8;
using namespace node;

namespace sail {

Dispatcher *globalAnemonodeDispatcher = nullptr;

}  // namespace sail

namespace {

class MonotonicClockDispatcher : public Dispatcher {
  public:
    virtual TimeStamp currentTime() { return MonotonicClock::now(); }
};

NAN_METHOD(adjTime) {
  Nan::HandleScope scope;

  if (!info[0]->IsNumber()) {
    Nan::ThrowTypeError("Expecting time delta in seconds");
    return;
  }
  double delta = info[0]->ToNumber()->Value();
  struct timeval tv;
  gettimeofday(&tv, 0);
  double secs = double(tv.tv_sec) + double(tv.tv_usec)*1e-6 + delta;
  tv.tv_sec = time_t(secs);
  tv.tv_usec = (secs - tv.tv_sec) * 1e6;
  int r = settimeofday(&tv, 0);
  if (r) {
    perror("settimeofday");
  }
  info.GetReturnValue().Set(r);
}

NAN_METHOD(currentTime) {
  Nan::HandleScope scope;

  if (globalAnemonodeDispatcher) {
    TimeStamp now = globalAnemonodeDispatcher->currentTime();

    info.GetReturnValue().Set(Nan::New<Date>(now.toMilliSecondsSince1970()).ToLocalChecked());
  } else {
    return;
  }
}

}  // namespace

void RegisterModule(Handle<Object> target) {
  Dispatcher *dispatcher = new MonotonicClockDispatcher();
  globalAnemonodeDispatcher = dispatcher;

  JsDispatcher::Init(dispatcher, target);
  JsNmea0183Source::Init(target);
  JsNmea2000Source::Init(target);
  JsLogger::Init(target);
  JsEstimator::Init(target);

  Nan::SetMethod(target, "adjTime", adjTime);
  Nan::SetMethod(target, "currentTime", currentTime);
}

// Register the module with node. Note that "modulename" must be the same as
// the basename of the resulting .node file. You can specify that name in
// binding.gyp ("target_name"). When you change it there, change it here too.
NODE_MODULE(anemonode, RegisterModule);
