#include <node.h>

#include <device/anemobox/Dispatcher.h>
#include <device/anemobox/Nmea0183Source.h>

#include <iostream>

#include <fcntl.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <unistd.h>

using namespace sail;
using namespace v8;

class PrintAngleListenener : public Listener<Angle<double>> {
 public:
  PrintAngleListenener(std::string prefix) : _prefix(prefix) { }
  virtual void onNewValue(const ValueDispatcher<Angle<double>> &dispatcher) {
    std::cout << _prefix << ": " << dispatcher.lastValue().degrees() << std::endl;
  }
 private:
  std::string _prefix;
};

bool run(const char *filename) {
  Dispatcher dispatcher;
  Nmea0183Source nmea0183(&dispatcher);
  PrintAngleListenener twaPrint("TWA");
  dispatcher.twa()->subscribe(&twaPrint);

  if (!nmea0183.open(filename)) {
    perror(filename);
    return false;
  }

  struct pollfd pfds[1];
  pfds[0].fd = nmea0183.fd();
  pfds[0].events = POLLIN;

  while (1) {
    int r = poll(pfds, sizeof(pfds) / sizeof(pfds[0]), -1);
    if (r >= 0) {
      nmea0183.poll();
    } else {
      if (errno != EAGAIN) {
        perror("poll");
        break;
      }
    }
  }
  return true;
}

Handle<Value> Run(const Arguments& args) {
  String::Utf8Value param1(args[0]->ToString());
  std::string path = std::string(*param1); 
  return Boolean::New(run(path.c_str()));
}

void RegisterModule(v8::Handle<v8::Object> target) {
    target->Set(String::NewSymbol("run"),
        FunctionTemplate::New(Run)->GetFunction());
}

// Register the module with node. Note that "modulename" must be the same as
// the basename of the resulting .node file. You can specify that name in
// binding.gyp ("target_name"). When you change it there, change it here too.
NODE_MODULE(anemonode, RegisterModule);
